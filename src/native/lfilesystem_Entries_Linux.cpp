/*
 * ======================================================================================
 *  __    ____  __  __  ____  ___
 * (  )  (_  _)(  \/  )( ___)/ __)
 *  )(__  _)(_  )    (  )__) \__ \
 * (____)(____)(_/\/\_)(____)(___/
 *
 *  This file is part of the Limes open source library and is licensed under the terms of the GNU Public License.
 *
 *  Commercial licenses are available; contact the maintainers at ben.the.vining@gmail.com to inquire for details.
 *
 * ======================================================================================
 */

#if ! (LIMES_LINUX || LIMES_ANDROID)
#	error
#endif

#if LIMES_ANDROID
#	include <fcntl.h>
#	include <sys/mman.h>
#endif

#if __has_include(<linux/limits.h>)
#	include <linux/limits.h>
#else
#	include <climits>
#endif

#include <unistd.h>	 // NOLINT
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>
#include <cinttypes>
#include <vector>
#include <array>
#include <atomic>

#include "lfilesystem/lfilesystem_CFile.h"
#include "lfilesystem/lfilesystem_Misc.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"
#include "lfilesystem/lfilesystem_File.h"
#include "lfilesystem/lfilesystem_Directory.h"

namespace limes::files
{

bool FilesystemEntry::isHidden() const
{
	return filenameBeginsWithDot();
}

bool FilesystemEntry::moveToTrash() noexcept
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	if (! exists())
		return false;

	Directory trashDir { "~/.Trash" };

	if (! trashDir.exists())
		trashDir = Directory { "~/.local/share/Trash/files" };

	if (! trashDir.exists())
		return false;

	return rename (trashDir.getChildFile (getName(), false).getAbsolutePath());
#endif
}

// TODO
bool FilesystemEntry::revealToUserInFileBrowser() const
{
	return false;
}

namespace exec_path
{

[[nodiscard]] LFILE_EXPORT std::string get_impl()
{
	std::array<char, maxPathLength()> buffer;

#if defined(__sun)
	static constexpr auto proc_self_exe = "/proc/self/path/a.out";
#else
	static constexpr auto proc_self_exe = "/proc/self/exe";
#endif

	if (const auto* resolved = realpath (proc_self_exe, buffer.data()))	 // NOLINT
	{
		return { resolved };
	}

	return {};
}

}  // namespace exec_path

namespace module_path
{

[[nodiscard]] LIMES_EXPORT std::string get_impl()
{
#ifdef _MSC_VER
#	define limes_get_return_address() _ReturnAddress()	 // NOLINT
#elif defined(__GNUC__)
#	define limes_get_return_address() __builtin_extract_return_addr (__builtin_return_address (0))	 // NOLINT
#endif

#ifdef limes_get_return_address

#	if defined(__sun)
	static constexpr auto proc_self_maps = "/proc/self/map";
#	else
	static constexpr auto proc_self_maps = "/proc/self/maps";
#	endif

	// try this several times, it may fail spuriously
	for (auto r = 0; r < 5; ++r)
	{
		const CFile selfMapFile { proc_self_maps, CFile::Mode::Read };

		if (! selfMapFile.isOpen())
			continue;

		std::array<char, std::max (std::uintmax_t (1024), maxPathLength())> buffer {};

		if (std::fgets (buffer.data(), sizeof (buffer), selfMapFile.get()) != nullptr)
			continue;

		std::array<char, 5> perms {};

		std::uint64_t low, high, offset;	// NOLINT
		std::uint32_t major, minor, inode;	// NOLINT

		std::array<char, maxPathLength()> path {};

		// TODO: don't use sscanf
		if (std::sscanf (buffer.data(), "%" PRIx64 "-%" PRIx64 " %s %" PRIx64 " %x:%x %u %s\n", &low, &high, perms.data(), &offset, &major, &minor, &inode, path.data()) != 8)	// NOLINT
			continue;

		const auto addr = reinterpret_cast<std::uintptr_t> (limes_get_return_address());  // NOLINT

		if (! (low <= addr && addr <= high))
			continue;

		if (const auto* resolved = realpath (path.data(), buffer.data()))  // NOLINT
		{
			auto length = static_cast<int> (std::strlen (resolved));

#	if LIMES_ANDROID
			if (length > 4
				&& buffer[length - 1] == 'k'
				&& buffer[length - 2] == 'p'
				&& buffer[length - 3] == 'a'
				&& buffer[length - 4] == '.')
			{
				const auto fd = open (path, O_RDONLY);

				if (fd == -1)
					continue;

				const auto* const begin = static_cast<char*> (mmap (0, offset, PROT_READ, MAP_SHARED, fd, 0));

				if (begin == MAP_FAILED)
				{
					close (fd);
					continue;
				}

				for (auto* p = begin + offset - 30;	 // minimum size of local file header
					 p >= begin;
					 --p)
				{
					if (*reinterpret_cast<std::uint32_t*> (p) == 0x04034b50UL)	// local file header signature found
					{
						const auto length_ = *(reinterpret_cast<std::uint32_t*> (p + 26));

						if (length + 2 + length_ < static_cast<std::uint32_t> (sizeof (buffer)))
						{
							std::memcpy (&buffer[length], "!/", 2);
							std::memcpy (&buffer[length + 2], p + 30, length_);
							length += 2 + length_;
						}

						continue;
					}
				}

				munmap (begin, offset);
				close (fd);
			}
#	endif /* LIMES_ANDROID */

			return { resolved, static_cast<std::string::size_type> (length) };
		}
	}  // close for loop

	return {};

#	undef limes_get_return_address

#else /* defined limes_get_return_address */
	return {};
#endif
}

}  // namespace module_path

}  // namespace files
