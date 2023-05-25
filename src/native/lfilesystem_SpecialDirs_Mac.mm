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

#import <Foundation/Foundation.h>
#include "lfilesystem/lfilesystem_SpecialDirectories.h"

namespace limes::files::dirs
{

[[nodiscard]] static inline std::string findSystemLocation (NSSearchPathDirectory type)
{
	if (auto* str = [NSSearchPathForDirectoriesInDomains (type, NSUserDomainMask, YES)
			objectAtIndex:0])
	{
		return std::string { [str UTF8String] };
	}

	return {};
}

Directory commonDocuments()
{
	return Directory { "/Users/Shared" };
}

Directory apps()
{
	@autoreleasepool
	{
		return Directory { findSystemLocation (NSApplicationDirectory) };
	}
}

Directory userAppData()
{
	@autoreleasepool
	{
		return Directory { findSystemLocation (NSLibraryDirectory) };
	}
}

Directory commonAppData()
{
	return userAppData();  // ??
}

Directory downloads()
{
	@autoreleasepool
	{
		return Directory { findSystemLocation (NSDownloadsDirectory) };
	}
}

Directory desktop()
{
	@autoreleasepool
	{
		return Directory { findSystemLocation (NSDesktopDirectory) };
	}
}

Directory userDocuments()
{
	@autoreleasepool
	{
		return Directory { findSystemLocation (NSDocumentDirectory) };
	}
}

}
