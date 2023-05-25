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

#pragma once

#if LIMES_WINDOWS
#	include <windows.h>
#elif __has_include(<linux/limits.h>)
#	include <linux/limits.h>
#else
#	include <climits>
#endif

#include <cstdint>
#include <filesystem>

/** @file
	This file defines miscellaneous filesystem utility functions.

	@ingroup files
 */

namespace limes::files
{

/** @ingroup files
	@{
 */

/** Returns the platform's preferred directory separator: \c \ on Windows, and \c / everywhere else. */
[[nodiscard]] LFILE_EXPORT constexpr char dirSeparator() noexcept
{
	return static_cast<char> (std::filesystem::path::preferred_separator);
}

/** Returns the platform's separator char for the \c PATH environment variable: \c ; on Windows, and \c : everywhere else.

	@todo is there a way to actually query the underlying filesystem for this?
 */
[[nodiscard]] LFILE_EXPORT consteval char PATHseparator() noexcept
{
#if LIMES_WINDOWS
	return ';';
#else
	return ':';
#endif
}

/** Returns true if the current platform's filesystem is likely to be case-sensitive.

	This is a compile-time determination based on platform conventions -- this function returns true on Linux, and false
	everywhere else. For a more accurate runtime query of the actual filesystem, see \c Volume::isCaseSensitive() and
	\c volume::caseSensitive().

	@see volume::caseSensitive(), Volume::isCaseSensitive()
 */
[[nodiscard]] LFILE_EXPORT consteval bool filesystemIsCaseSensitive() noexcept
{
#if LIMES_LINUX
	return true;
#else
	return false;
#endif
}

/** Returns the maximum path length possible on the current operating system.
	This is a compile-time constant defined by platform headers.

	@todo query the actual underlying filesystem for this?
 */
[[nodiscard]] LFILE_EXPORT constexpr std::uintmax_t maxPathLength() noexcept
{
#if LIMES_WINDOWS
	return static_cast<std::uintmax_t> (MAX_PATH);
#elif defined(NAME_MAX)
	return static_cast<std::uintmax_t> (NAME_MAX);
#elif defined(PATH_MAX)
	return static_cast<std::uintmax_t> (PATH_MAX);
#elif defined(FILENAME_MAX)
	return static_cast<std::uintmax_t> (FILENAME_MAX);
#else
#	error "Cannot detect maximum path length for your platform!"
#endif
}

/** @}*/

}  // namespace files
