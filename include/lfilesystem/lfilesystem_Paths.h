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

#include <filesystem>
#include "lfilesystem/lfilesystem_Export.h"

/** @file
	This file defines some utility functions for working with paths.
	@ingroup limes_files
 */

namespace limes::files
{

using Path = std::filesystem::path;

/** Tests whether the passed path contains any invalid characters or character
	sequences, such as \c :: . This also returns false if the path is longer
	than \c maxPathLength() . This returns false if the passed path is empty.

	@see normalizePath()
	@ingroup limes_files
 */
[[nodiscard]] LFILE_EXPORT bool isValidPath (const Path& path);

/** This function takes an input path and does some linting and transformations
	to create a consistent, canonical form of the path. If \c isValidPath()
	returns false for the input path, this function returns an empty path.

	Any trailing directory separators will be removed. Path segments of the
	form \c /./ will be normalized to \c / . This is also true of \c .. --
	a path of the form \c some/rel/../path will be normalized to \c some/path .

	On non-Windows platforms, a tilde in the path will be expanded to the home
	directory, and paths of the form \c ~username will be expanded to the user
	\c username 's home directory.

	@see isValidPath()
	@ingroup limes_files
 */
[[nodiscard]] LFILE_EXPORT Path normalizePath (const Path& path);

/** Returns the largest prefix path fragment common to \c path1 and \c path2 .

	For example, if \c path1 is \c a/b/c/d and \c path2 is \c a/b/e/f
	then this will return \c a/b . Both paths will be normalized via
	\c normalizePath() before computing the common prefix.

	@ingroup limes_files
 */
[[nodiscard]] LFILE_EXPORT Path largestCommonPrefix (const Path& path1, const Path& path2);

}  // namespace limes::files
