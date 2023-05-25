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

#include "lfilesystem_Paths.h"

#if ! LIMES_WINDOWS
#	include <sys/types.h>
#	include <pwd.h>
#	include "lfilesystem_SpecialDirectories.h"
#endif

#include <algorithm>
#include "lfilesystem_Misc.h"

namespace limes::files
{

bool isValidPath (const Path& path)
{
	if (path.empty())
		return false;

	const auto str = path.string();

	if (str::contains (str, "::"))
		return false;

	if (str.length() > maxPathLength())
		return false;

	return true;
}

#if ! LIMES_WINDOWS

[[nodiscard]] static inline Path expandTilde (const std::string& path)
{
	const auto homePath = dirs::home().getAbsolutePath();

	if (path.length() == 1)
		return homePath;

	if (path[1] == dirSeparator() || path[1] == 0)
	{
		// the path is in the form "~/abc"
		return homePath / path.substr (2, std::string::npos);
	}

	// the path is in the form "~user/abc"

	const auto afterTilde	 = path.substr (1, std::string::npos);
	const auto userName		 = str::upToFirstOccurrenceOf (afterTilde, "/");
	const auto afterUsername = str::fromFirstOccurrenceOf (afterTilde, "/");

	if (const auto* userInfo = getpwnam (userName.c_str()))
	{
		const Path userHome { std::string { userInfo->pw_dir } };

		return userHome / afterUsername;
	}

	return homePath / afterUsername;
}

#endif /* ! LIMES_WINDOWS */

static inline void normalizeDoubleDot (std::string& path)
{
	if (path == std::string { ".." })
		return;

	while (str::contains (path, ".."))
	{
		// normalize dir separators to / to make this operation easier
		const auto canonical = str::replace (path, "\\", "/");

		auto	   before = str::upToFirstOccurrenceOf (canonical, "..");
		const auto after  = str::fromFirstOccurrenceOf (canonical, "..");

		if (! after.empty() && ! after.starts_with ('/'))
			return;

		// do this twice, because we want the second-to-last directory separator
		before = str::upToLastOccurrenceOf (before, "/");
		before = str::upToLastOccurrenceOf (before, "/");

		const auto firstPart  = path.substr (0, before.length());
		const auto secondPart = path.substr (path.length() - after.length(), std::string::npos);

		path = firstPart + after;
	}
}

static inline void removeTrailingDirSeparators (std::string& path)
{
	while (path.ends_with ('/'))
		path.pop_back();

#if LIMES_WINDOWS
	while (path.ends_with ('\\'))
		path.pop_back();
#endif
}

static inline void normalizeSlashDotSlash (std::string& path)
{
	path = str::replace (path, "/./", "/");

#if LIMES_WINDOWS
	path = str::replace (path, "\\.\\", "\\");
#endif
}

static inline void normalizeDotSlash (std::string& path)
{
	if (path == std::string { "./" })
	{
		path = '.';
		return;
	}

	if (path.starts_with ("./"))
		path = path.substr (2, std::string::npos);

#if LIMES_WINDOWS
	if (path == std::string { ".\\" })
	{
		path = '.';
		return;
	}

	if (path.starts_with (".\\"))
		path = path.substr (2, std::string::npos);
#endif
}

[[nodiscard]] static inline bool isOnlyDirectorySeparator (const std::string& path)
{
#if LIMES_WINDOWS
	if (path == std::string { '\\' })
		return true;
#endif

	return path == std::string { '/' };
}

// return an empty string for an invalid path
// always remove all trailing /'s
// always remove all trailing \'s (Windows only)
// replace all /./ with /
// replace all \.\ with \ (Windows only)
// resolve /../
// expand a leading ~ (non-Windows only)
Path normalizePath (const Path& path)
{
	if (! isValidPath (path))
		return Path {};

	auto str = path.string();

	if (isOnlyDirectorySeparator (str))
		return path;

	removeTrailingDirSeparators (str);

	// remove a trailing .
	if (str.length() > 1 && str.ends_with ('.') && ! str.ends_with (".."))
		str.pop_back();

	normalizeDotSlash (str);

	normalizeSlashDotSlash (str);

	normalizeDoubleDot (str);

#if ! LIMES_WINDOWS
	if (str.starts_with ('~'))
		return expandTilde (str);
#endif

	removeTrailingDirSeparators (str);

	return str;
}

Path largestCommonPrefix (const Path& path1, const Path& path2)
{
	const auto a = normalizePath (path1);
	const auto b = normalizePath (path2);

	if (a == b)
		return a;

	if (a.empty() || b.empty())
		return Path {};

	const auto aStr = a.string();
	const auto bStr = b.string();

	// TODO: support \ on Windows
	const auto aChunks = str::split (aStr, "/", false);
	const auto bChunks = str::split (bStr, "/", false);

	Path result;

	// make sure to preserve a leading /
	if (aStr.starts_with ('/') && bStr.starts_with ('/'))
		result = "/";

	for (auto i = 0UL; i < std::min (aChunks.size(), bChunks.size()); ++i)
	{
		// TODO: deal with case sensitivity
		if (aChunks[i] != bChunks[i])
			break;

		result /= aChunks[i];
	}

	return normalizePath (result);
}

}  // namespace files
