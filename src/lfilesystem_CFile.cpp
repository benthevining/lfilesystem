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

#include "lfilesystem_CFile.h"
#include <stdio.h>
#include <cstdio>
#include <string_view>

#if LIMES_WINDOWS
#	include <Windows.h>
#	include <io.h>
#	include <string>
#elif LIMES_APPLE
#	include <sys/fcntl.h>
#else
#	include <unistd.h>
#endif

#include "lfilesystem_File.h"
#include "lfilesystem_Paths.h"

namespace limes::files
{

CFile::CFile (const Path& filepath, Mode mode) noexcept
{
	open (filepath, mode);
}

CFile::CFile (std::FILE* fileHandle) noexcept
	: ptr (fileHandle)
{
}

CFile::~CFile() noexcept
{
	close();
}

CFile::CFile (CFile&& other) noexcept
	: ptr (other.ptr)
{
	other.ptr = nullptr;
}

CFile& CFile::operator= (CFile&& other) noexcept
{
	close();

	ptr		  = other.ptr;
	other.ptr = nullptr;

	return *this;
}

std::FILE* CFile::get() const noexcept
{
	return ptr;
}

std::FILE* CFile::operator->() const noexcept
{
	return ptr;
}

std::FILE& CFile::operator*() const
{
	LIMES_ASSERT (ptr != nullptr);
	return *ptr;
}

CFile::operator std::FILE*() const noexcept
{
	return ptr;
}

[[nodiscard]] static inline std::string_view modeToString (CFile::Mode mode) noexcept
{
	switch (mode)
	{
		case (CFile::Mode::Read) : return "r";
		case (CFile::Mode::Write) : return "w";
		case (CFile::Mode::Append) : return "a";
		case (CFile::Mode::ReadExtended) : return "r+";
		case (CFile::Mode::WriteExtended) : return "w+";
		case (CFile::Mode::AppendExtended) : return "a+";
		default : LIMES_UNREACHABLE; return "";
	}
}

[[nodiscard]] static inline std::string pathToString (const Path& path)
{
	auto pathCopy { path };

	return pathCopy.make_preferred().string();
}

bool CFile::open (const Path& filepath, Mode mode) noexcept
{
	close();

	try
	{
		ptr = std::fopen (pathToString (filepath).c_str(), modeToString (mode).data());

		return ptr != nullptr;
	}
	catch (...)
	{
		close();

		return false;
	}
}

void CFile::close() noexcept
{
	if (ptr != nullptr)
	{
		try
		{
			std::fclose (ptr);
		}
		catch (...)
		{
		}
	}
}

bool CFile::isOpen() const noexcept
{
	return ptr != nullptr;
}

CFile::operator bool() const noexcept
{
	return ptr != nullptr;
}

Path CFile::getPath() const
{
	if (ptr == nullptr)
		return {};

#if LIMES_WINDOWS

	char buf[MAX_PATH] = {};

	if (GetFinalPathNameByHandleA (reinterpret_cast<HANDLE> (_get_osfhandle (_fileno (ptr))),
								   buf,
								   static_cast<DWORD> (MAX_PATH),
								   FILE_NAME_OPENED)
		== 0)
	{
		return {};
	}

	std::string path { buf };

	// some of the paths returned by GetFinalPathNameByHandleA are prefixed with \\?\

	if (path.starts_with ("\\\\?\\"))
		path = path.substr (4, std::string::npos);

	return normalizePath (path);

#elif LIMES_APPLE

	char buf[PATH_MAX];

	if (fcntl (fileno (ptr), F_GETPATH, buf) < 0)
	{
		return {};
	}

	return normalizePath (Path { buf });

#else

	char buf[PATH_MAX];
	char filenameBuf[sizeof "/prof/self/fd/0123456789"];

	sprintf (filenameBuf, "/proc/self/fd/%d", fileno (ptr));  // NOLINT

	const auto num = readlink (filenameBuf, buf, sizeof (buf));

	if (num < 0)
		return {};

	buf[num] = '\0';

	return normalizePath (Path { buf });

#endif
}

File CFile::getFile() const
{
	return File { getPath() };
}

CFile CFile::createTempFile()
{
	return CFile { std::tmpfile() };
}

}  // namespace limes::files
