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

#include <dlfcn.h>		  // for dladdr, Dl_info
#include <mach-o/dyld.h>  // for _NSGetExecutablePath
#include <limits.h>		  // for PATH_MAX

#import <Foundation/Foundation.h>
#include <TargetConditionals.h>

#if ! TARGET_OS_IPHONE
#	import <AppKit/AppKit.h>
#endif

#include <cstdint>	// for uint32_t
#include <cstdlib>	// for free, realpath, malloc
#include <cstring>	// for strlen
#include <string>	// for string
#include <array>
#include <atomic>
#include <thread>

#include "lfilesystem/lfilesystem_Misc.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"
#include "lfilesystem/lfilesystem_File.h"

namespace limes::files
{

[[maybe_unused]] LFILE_NO_EXPORT static inline NSURL* path_to_nsurl (const Path& p) noexcept
{
	if (auto* str = [NSString stringWithUTF8String:p.c_str()])
		return [NSURL fileURLWithPath:str];

	return nullptr;
}

bool FilesystemEntry::isHidden() const
{
#if TARGET_OS_IPHONE
	return filenameBeginsWithDot();
#else
	if (! exists())
		return filenameBeginsWithDot();

	@autoreleasepool
	{
		NSNumber* hidden = nil;
		NSError*  err	 = nil;

		if (auto* url = path_to_nsurl (getAbsolutePath()))
			return [url getResourceValue:&hidden forKey:NSURLIsHiddenKey error:&err]
				&& [hidden boolValue];

		return filenameBeginsWithDot();
	}
#endif
}

bool FilesystemEntry::moveToTrash() noexcept
{
	if (! exists())
		return false;

#if ! TARGET_OS_TV
	@autoreleasepool
	{
		if (@available (macOS 10.8, iOS 11.0, *))
		{
			NSError* error = nil;

			if (auto* url = path_to_nsurl (getAbsolutePath()))
				return [[NSFileManager defaultManager] trashItemAtURL:url
													 resultingItemURL:nil
																error:&error];
		}
	}
#endif

	return deleteIfExists();
}

bool FilesystemEntry::revealToUserInFileBrowser() const
{
#if TARGET_OS_IPHONE
	return false;
#else
	if (! exists())
		return false;

	[[NSWorkspace sharedWorkspace] selectFile:[NSString stringWithUTF8String:getAbsolutePath().c_str()] inFileViewerRootedAtPath:[NSString string]];

	return true;
#endif
}

#if ! TARGET_OS_IPHONE
bool File::isMacOSBundle() const noexcept
{
	@autoreleasepool
	{
		return [[NSWorkspace sharedWorkspace] isFilePackageAtPath:[NSString stringWithUTF8String:getAbsolutePath().c_str()]];
	}
}
#endif

namespace exec_path
{

[[nodiscard]] LFILE_EXPORT std::string get_impl()
{
	std::array<char, maxPathLength()> buffer1 {};
	std::array<char, maxPathLength()> buffer2 {};

	auto* path = buffer1.data();

	auto size = static_cast<std::uint32_t> (sizeof (buffer1));

	if (_NSGetExecutablePath (path, &size) == -1)
	{
		// try again, with a larger buffer for the path

		path = static_cast<char*> (std::malloc (size));

		if (! _NSGetExecutablePath (path, &size))
		{
			if (path != buffer1.data())
				std::free (path);

			return {};
		}
	}

	if (const auto* resolved = realpath (path, buffer2.data()))
	{
		if (path != buffer1.data())
			std::free (path);

		return { resolved };
	}

	if (path != buffer1.data())
		std::free (path);

	return {};
}

}

namespace module_path
{

[[nodiscard]] LFILE_EXPORT std::string get_impl()
{
#ifdef _MSC_VER
#	define limes_get_return_address() _ReturnAddress()	 // NOLINT
#elif defined(__GNUC__)
#	define limes_get_return_address() __builtin_extract_return_addr (__builtin_return_address (0))	 // NOLINT
#endif

#ifdef limes_get_return_address

	Dl_info info;

	if (dladdr (limes_get_return_address(), &info))
	{
		std::array<char, maxPathLength()> buffer {};

		if (const auto* resolved = realpath (info.dli_fname, buffer.data()))
		{
			return { resolved };
		}
	}

	return {};

#	undef limes_get_return_address

#else
	return {};
#endif
}

}

}  // namespace files
