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

#include <vector>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_Directory.h"

/** @defgroup limes_files_special_dirs Special directories
	Utility functions for finding some common directory locations.

	@ingroup limes_files

	@todo more dirs: cache, pictures, music, video

	@todo Android implementations. None of these functions are implemented for Android, it will
	probably require some native JNI code.
 */

/** @file
	This file contains utility functions for finding some common directories.

	@ingroup limes_files_special_dirs
	@see directory.h
 */

/** This namespace contains utility functions for finding some common directories.

	@see files::Directory
	@ingroup limes_files_special_dirs
 */
namespace limes::files::dirs
{

/** @ingroup limes_files_special_dirs
	@{
 */

/** Returns the system's current working directory.

	@see Directory
	@relates Directory

	This function is not marked pure because it depends on the current system environment.

	@see cwdAtStartup(), setCWD()
 */
[[nodiscard]] LFILE_EXPORT Directory cwd();

/** Returns the program's working directory when it was first started up.

	Technically, this returns the current working directory from when the Limes code's static
	storage duration variables were initialized.

	@see Directory
	@relates Directory

	@see cwd()
 */
[[nodiscard]] LFILE_EXPORT Directory cwdAtStartup();

/** Sets the system's current working directory.

	@returns False if the passed path was already the current working directory, or if the passed path
	does not exist or is not an absolute path.

	@see cwd()
	@relates Directory
 */
LFILE_EXPORT bool setCWD (const Path& path);

/** Returns a directory appropriate for storing temporary files.

	@see Directory, TempFile
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory temp();

/** @envvar @b PATH This standard environment variable contains paths that are searched for
	executables by the runtime loader. This variable's contents can be retrieved using the
	\c files::dirs::PATH() function, and the separator character used to delimit individual
	paths within its content can be retrieved with the function files::PATHseparator() .
 */

/** Returns the contents of the \c PATH environment variable as a vector of Directory objects.

	@see Directory, appendToPATH()
	@relates Directory

	@internal
	This function is not marked pure because it depends on the current value of the environment
	variable \c PATH.
	@endinternal
 */
[[nodiscard]] LFILE_EXPORT std::vector<Directory> PATH();

/** Appends the path of the passed directory to the contents of the \c PATH environment variable.

	@returns False if the passed directory was already in \c PATH ; otherwise, true if setting
	the \c PATH environment variable succeeds.

	@see PATH()
	@relates Directory
 */
LFILE_EXPORT bool appendToPATH (const Directory& dir);

/** Returns the current user's home directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory home();

/** Returns the current user's desktop directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory desktop();

/** Returns the current user's documents directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory userDocuments();

/** Returns the system-wide documents directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory commonDocuments();

/** Returns the current user's application data directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory userAppData();

/** Returns the system-wide application data directory.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory commonAppData();

/** Returns the directory where apps are typically installed.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory apps();

/** Returns the directory where the user's downloads are stored.

	@see Directory
	@relates Directory
 */
[[nodiscard]] LFILE_EXPORT Directory downloads();

/** @} */

}  // namespace limes::files::dirs
