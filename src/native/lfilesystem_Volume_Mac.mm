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

#if ! LIMES_APPLE
#	error
#endif

#include <Foundation/Foundation.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/attr.h>
#include <sys/mount.h>
#include <unistd.h>

#if LIMES_OSX
#	import <AppKit/AppKit.h>
#endif

#include <sstream>
#include <stdexcept>
#include <string_view>
#include "lfilesystem_Volume.h"
#include "lfilesystem_Misc.h"

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
	struct stat fileStat;

	if (stat (inputPath.c_str(), &fileStat) != 0)
		throwError (inputPath);

	struct statfs* mounts { nullptr };

	const auto defer = func::defer_call ([mounts]
										 { delete[] mounts; });

	const auto numMounts = getmntinfo (&mounts, MNT_WAIT);

	if (numMounts < 1)
		throwError (inputPath);

	for (auto i = 0; i < numMounts; ++i)
		if (fileStat.st_dev == mounts[i].f_fsid.val[0])
			return Path { mounts[i].f_mntonname };

	throwError (inputPath);
}

Volume::Volume (const Path& path)
	: rootPath (findMountPath (path))
{
}

std::string Volume::getLabel() const
{
#if ! LIMES_IOS
	struct VolAttrBuf
	{
		u_int32_t		length;
		attrreference_t mountPointRef;
		char			mountPointSpace[MAXPATHLEN];
	} attrBuf;

	struct attrlist attrList;

	attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
	attrList.volattr	 = ATTR_VOL_INFO | ATTR_VOL_NAME;

	if (getattrlist (rootPath.c_str(), &attrList, &attrBuf, sizeof (attrBuf), 0) == 0)
	{
		return { ((const char*) &attrBuf.mountPointRef) + attrBuf.mountPointRef.attr_dataoffset,
				 static_cast<std::string::size_type> (attrBuf.mountPointRef.attr_length) };
	}
#endif

	// TODO: is this completely unsupported on iOS?

	return {};
}

// TODO!
int Volume::getSerialNumber() const
{
	return 0;
}

// essentially a backup function for getType() if stat fails
static inline bool isRemovableVolume ([[maybe_unused]] const Path& path)
{
#if LIMES_IOS
	return false;  // is it possible for this to be true on iOS, tvOS, or watchOS?
#else
	@autoreleasepool
	{
		BOOL removable = false;

		[[NSWorkspace sharedWorkspace]
			getFileSystemInfoForPath:[NSString stringWithCString:path.c_str() encoding:[NSString defaultCStringEncoding]]
						 isRemovable:&removable
						  isWritable:nil
					   isUnmountable:nil
						 description:nil
								type:nil];

		return removable;
	}
#endif
}

// TODO: detect network & RAM types
Volume::Type Volume::getType() const
{
#if LIMES_IOS
	static constexpr auto defaultUnknownType = Type::HardDisk;	// assume hard disk if we're on iOS
#else
	static constexpr auto defaultUnknownType = Type::Unknown;  // can't make any assumptions if we're on desktop
#endif

	struct statfs info;

	// first try stat
	// if stat fails, then use the less granular isRemovableVolume()
	if (statfs (rootPath.c_str(), &info) != 0)
	{
		if (isRemovableVolume (rootPath))
			return Type::Removable;

		return defaultUnknownType;
	}

	const std::string_view typeName { info.f_fstypename };

	if (typeName == "cd9660" || typeName == "cdfs" || typeName == "cddafs" || typeName == "udf")
		return Type::CDRom;

	if (typeName == "nfs" || typeName == "hfs" || typeName == "smbfs" || typeName == "ramfs" || typeName == "apfs")
		return Type::HardDisk;

	if (isRemovableVolume (rootPath))
		return Type::Removable;

	return defaultUnknownType;
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
	@autoreleasepool
	{
		auto* filesystem = [NSURL fileURLWithPath:[NSString stringWithCString:rootPath.c_str()
																	 encoding:[NSString defaultCStringEncoding]]
									  isDirectory:YES];

		NSNumber* caseSensitiveFS;

		const auto canDetect = [filesystem getResourceValue:&caseSensitiveFS
													 forKey:NSURLVolumeSupportsCaseSensitiveNamesKey
													  error:NULL];

		if (! canDetect)
			return filesystemIsCaseSensitive();

		return [caseSensitiveFS intValue] == 1;
	}
}

std::vector<Volume> Volume::getAll() noexcept
{
	std::vector<Volume> volumes;

	struct statfs* mounts { nullptr };

	try
	{
		const auto numMounts = getmntinfo (&mounts, MNT_WAIT);

		if (numMounts < 1)
		{
			delete[] mounts;
			return volumes;
		}

		for (auto i = 0; i < numMounts; ++i)
		{
			const Path path { mounts[i].f_mntonname };

			volumes.emplace_back (Volume { path });
		}
	}
	catch (...)
	{
	}

	delete[] mounts;

	return volumes;
}

}
