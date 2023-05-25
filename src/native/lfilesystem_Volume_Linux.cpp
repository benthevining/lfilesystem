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

#if ! LIMES_LINUX
#	error
#endif

#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <mntent.h>

#if __has_include(<linux/limits.h>)
#	include <linux/limits.h>
#else
#	include <climits>
#endif

#if defined(__linux__)
#	include <linux/hdreg.h>
#	include <sys/ioctl.h>
#	include <fcntl.h>
#	include <string>
#endif

#include <sstream>
#include <stdexcept>
#include <cstdio>
#include <vector>
#include <cctype>
#include <algorithm>
#include "lfilesystem_Directory.h"
#include "lfilesystem_File.h"
#include "lfilesystem_Volume.h"

#ifndef LFILE_IMPL_USE_PATHCONF
#	define LFILE_IMPL_USE_PATHCONF 0
#endif

#if LFILE_IMPL_USE_PATHCONF
#	include <unistd.h>
#else
#	include "lfilesystem_Misc.h"
#endif

namespace limes::files
{

[[noreturn]] static inline void throwError (const Path& inputPath)
{
	std::stringstream stream;

	stream << "Volume path cannot be determined for path "
		   << inputPath.string();

	throw std::runtime_error { stream.str() };
}

static inline Path findMountPath (const Path& inputPath)
{
	struct stat s;

	if (stat (inputPath.c_str(), &s) != 0)
		throwError (inputPath);

	if (auto* fp = setmntent ("/proc/mounts", "r"))	 // should this be etc/mtab ??
	{
		struct mntent mnt;

		char buf[PATH_MAX] = {};

		const auto dev = s.st_dev;

		while (getmntent_r (fp, &mnt, buf, PATH_MAX) != nullptr)
		{
			if (stat (mnt.mnt_dir, &s) != 0)
				continue;

			if (s.st_dev == dev)
			{
				endmntent (fp);
				return Path { mnt.mnt_dir };
			}
		}

		endmntent (fp);
	}

	throwError (inputPath);
}

Volume::Volume (const Path& path)
	: rootPath (findMountPath (path))
{
}

std::string Volume::getLabel() const
{
	// loop through all files in /dev/disk/by-label
	// stat each of them and find the one whose inode
	// is the same as the inode for the volume's
	// logical path

	const Directory byLabel { "/dev/disk/by-label" };

	if (! byLabel.exists())
		return {};

	struct stat volumeStat;

	if (stat (rootPath.c_str(), &volumeStat) != 0)
		return {};

	const auto volumeInode = volumeStat.st_ino;

	for (const auto& file : byLabel.getChildFiles (true))
	{
		struct stat fileStat;

		if (stat (file.getAbsolutePath().c_str(), &fileStat) != 0)
			continue;

		if (fileStat.st_ino == volumeInode)
			return file.getFilename (false);
	}

	return {};
}

int Volume::getSerialNumber() const
{
#if defined(__linux__)
	// TODO: this may only work with root privileges...
	const auto fd = open ("/dev/hda", O_RDONLY | O_NONBLOCK);

	if (fd < 0)
		return 0;

	struct hd_driveid hd;

	if (ioctl (fd, HDIO_GET_IDENTITY, &hd) < 0)
		return 0;

	const std::string num { reinterpret_cast<const char*> (&hd.serial_no[0]), 20 };

	return std::stoi (num);
#else
	return 0;
#endif
}

// TODO: detect RAM type?
Volume::Type Volume::getType() const
{
	struct statfs buf;

	if (statfs (rootPath.c_str(), &buf) != 0)
		return Type::Unknown;

	enum
	{
		U_ISOFS_SUPER_MAGIC = 0x9660,  // linux/iso_fs.h
		U_MSDOS_SUPER_MAGIC = 0x4d44,  // linux/msdos_fs.h
		U_NFS_SUPER_MAGIC	= 0x6969,  // linux/nfs_fs.h
		U_SMB_SUPER_MAGIC	= 0x517B   // linux/smb_fs.h
	};

	switch (buf.f_type)
	{
		case (U_ISOFS_SUPER_MAGIC) : return Type::CDRom;
		case (U_MSDOS_SUPER_MAGIC) : return Type::Removable;
		case (U_NFS_SUPER_MAGIC) : return Type::Network;
		case (U_SMB_SUPER_MAGIC) : return Type::Network;
		default : return Type::HardDisk;
	}
}

bool Volume::isReadOnly() const
{
	struct statvfs info;

	if (statvfs (rootPath.c_str(), &info) != 0)
		return false;

	return (info.f_flag & ST_RDONLY) != 0;
}

bool Volume::isCaseSensitive() const
{
#if LFILE_IMPL_USE_PATHCONF
	return pathconf (rootPath.c_str(), _PC_CASE_SENSITIVE) == 1L;
#else
	return filesystemIsCaseSensitive();
#endif
}

static inline void trimString (std::string& string)
{
	if (string.empty())
		return;

	// trim start
	string.erase (string.begin(), std::find_if (string.begin(), string.end(), [] (unsigned char ch)
												{ return ! std::isspace (ch); }));

	// trim end
	string.erase (std::find_if (string.rbegin(), string.rend(), [] (unsigned char ch)
								{ return ! std::isspace (ch); })
					  .base(),
				  string.end());
}

static inline std::vector<std::string> splitAtWhitespace (std::string_view stringToSplit)
{
	std::vector<std::string> tokens;

	auto tokenStart = stringToSplit.begin();
	auto pos		= tokenStart;

	while (pos != stringToSplit.end())
	{
		if (std::isspace (*pos))
		{
			auto delimiterStart = pos++;

			while (pos != stringToSplit.end() && std::isspace (*pos))
				++pos;

			if (pos != stringToSplit.begin())
				tokens.push_back ({ tokenStart, delimiterStart });

			tokenStart = pos;
		}
		else
		{
			++pos;
		}
	}

	if (pos != stringToSplit.begin())
		tokens.push_back ({ tokenStart, pos });

	for (auto& token : tokens)
		trimString (token);

	return tokens;
}

std::vector<Volume> Volume::getAll() noexcept
{
	// reads /etc/mtab to find all listed mount points

	std::vector<Volume> volumes;

	try
	{
		const File mtab { "/etc/mtab" };

		if (! mtab.exists())
			return volumes;

		for (auto& line : mtab.loadAsLines())
		{
			for (const auto& entry : splitAtWhitespace (line))
			{
				if (entry.starts_with ("/"))
				{
					const Path path { entry };

					volumes.emplace_back (Volume { path });
				}
			}
		}
	}
	catch (...)
	{
	}

	return volumes;
}

}  // namespace files
