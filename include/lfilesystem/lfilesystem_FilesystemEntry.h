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

#include <cstdint>	   // for uintmax_t
#include <filesystem>  // for copy_options, path, perm_options
#include <string>	   // for string
#include <functional>  // for std::hash
#include <optional>
#include <ostream>
#include "lfilesystem_Permissions.h"

/** @file
	This file defines the FilesystemEntry class.

	@ingroup files
 */

namespace limes::files
{

class Directory;
class File;
class SymLink;
class Volume;

/** Convenience typedef for filesystem paths.
	@ingroup files
 */
using Path = std::filesystem::path;

/** The base class for any kind of object on the filesystem.

	This class is essentially a wrapper around \c std::filesystem::path , with a more object-oriented design.
	This class holds only the path, and so is relatively trivial to construct or copy (think of it like a string).

	This class normalizes paths passed to it using the \c normalizePath() function.

	The kinds of filesystem objects Limes defines are File, Directory, and SymLink, but custom file-like
	object types can be created as well. To satisfy this interface, a custom type must have and be able to
	logically manipulate a \c std::filesystem::path .

	This base class contains various semantic operations on and queries of the path, some basic disk operations,
	and provides functions for converting any \c FilesystemEntry object to higher-level \c Directory , \c File ,
	or \c SymLink objects.

	For example:
	@code{.cpp}
	limes::files::FilesystemEntry file { "/path/to/some/file.txt" };

	assert (file.isFile());
	assert (! file.isDirectory());
	assert (file.isAbsolutePath());

	if (file.exists())
		std::cout << file.getFileObject()->loadAsString();
	@endcode

	@ingroup files
	@see File, Directory, SymLink

	@todo getNonexistentSibling()
	@todo getCreationTime(), getLastAccessTime()
	@todo tests for getModificationTime(), getCreationTime(), getLastAccessTime()
 */
class LFILE_EXPORT FilesystemEntry
{
public:
	/** @name Typedefs */
	///@{

	/** Options bitfield for setting permissions. */
	using PermOptions = std::filesystem::perm_options;

	/** Options bitfield for copying. */
	using CopyOptions = std::filesystem::copy_options;

	/** A time point used for filesystem time. */
	using Time = std::filesystem::file_time_type;

	///@}

	/** @name Constructors */
	///@{

	/** Creates a FilesystemEntry with an empty path.
		After creating a FilesystemEntry with this default constructor, calling \c isValid() on it will return false.
		You can use the \c assignPath() function to assign this object to a path after it has been constructed.
	 */
	FilesystemEntry() = default;

	/** Creates a FilesystemEntry referring to the specified path.

		@param pathToUse The path to this filesystem entry. This can be an absolute or relative path, and it is not
		an error if the passed path doesn't actually exist on disk. The path will be normalized using the
		\c normalizePath() function.
	 */
	explicit FilesystemEntry (const Path& pathToUse);

	///@}

	/** Destructor. */
	virtual ~FilesystemEntry() = default;

	FilesystemEntry (const FilesystemEntry&) = default;
	FilesystemEntry& operator= (const FilesystemEntry&) = default;

	FilesystemEntry (FilesystemEntry&&) = default;
	FilesystemEntry& operator= (FilesystemEntry&&) = default;

	/** @name Path assignment */
	///@{
	/** Assigns the path this filesystem entry represents.

		@note This has no effect on disk contents, this only updates the semantic path this object holds. To change the
		path on disk, see the \c rename() function. The path will be normalized using the \c normalizePath() function.

		@todo test coverage (for all 3)
	 */
	FilesystemEntry& operator= (const Path& newPath);
	FilesystemEntry& operator= (const std::string_view& newPath);
	FilesystemEntry& assignPath (const Path& newPath) noexcept;
	///@}

	/** @name Equality comparison */
	///@{
	/** Compares this filesystem entry's absolute path with the other one's for equality. Note that this is a path comparison,
		and does not compare the contents of files. This comparison attempts to respect the case sensitivity of the underlying
		filesystem.

		@todo test coverage for overloads accepting a Path argument
	 */
	[[nodiscard]] bool operator== (const FilesystemEntry& other) const noexcept;
	[[nodiscard]] bool operator== (const Path& other) const noexcept;
	[[nodiscard]] bool operator!= (const FilesystemEntry& other) const noexcept;
	[[nodiscard]] bool operator!= (const Path& other) const noexcept;
	///@}

	/** @name Lexicographic comparison */
	///@{
	/** Lexicographically compares this filesystem entry's absolute path with another path. Directories will always be sorted
		before other types of filesystem objects; other than this, \c std::filesystem::path 's built-in lexicographic sort
		is used.

		@todo test coverage
	 */
	[[nodiscard]] bool operator<(const FilesystemEntry& other) const noexcept;
	[[nodiscard]] bool operator> (const FilesystemEntry& other) const noexcept;
	[[nodiscard]] bool operator<(const Path& other) const noexcept;
	[[nodiscard]] bool operator> (const Path& other) const noexcept;
	///@}

	/** @name Path manipulation */
	///@{

	/** Returns a new FilesystemEntry referring to a subpath of this one.

		For example:
		@code{.cpp}
		const limes::files::FilesystemEntry dir { "/path/to/directory" };

		const auto child = dir / "file.txt";

		assert (child.getAbsolutePath() == "/path/to/directory/file.txt");
		@endcode

		@note This has no effect on disk contents, this only updates the semantic path this object holds. To change the path on
		disk, see the \c rename() function.

		@todo test coverage
	 */
	[[nodiscard]] FilesystemEntry operator/ (const std::string_view& subpathName) const;

	/** Assigns this object's path to a subpath of the current path.

		For example:
		@code{.cpp}
		limes::files::FilesystemEntry dir { "/path/to/directory" };

		dir /= "file.txt";

		assert (dir.getAbsolutePath() == "/path/to/directory/file.txt");
		@endcode

		@note This has no effect on disk contents, this only updates the semantic path this object holds. To change the path on
		disk, see the \c rename() function.

		@todo test coverage
	 */
	FilesystemEntry& operator/= (const std::string_view& subpathName);

	/** Changes the last section of the path (the filename).

		For example:
		@code{.cpp}
		limes::files::FilesystemEntry file { "/path/to/directory/file.txt" };

		file.changeName ("newName");

		assert (file.getAbsolutePath() == "/path/to/directory/newName.txt");
		@endcode

		@note This has no effect on disk contents, this only updates the semantic path this object holds. To change the path on
		disk, see the \c rename() function.
	 */
	FilesystemEntry& changeName (const std::string_view& newName);

	/** If the path this object holds is already absolute, this function does nothing and returns false. Otherwise, this function
		assigns the path to \c basePath/currentPath (prepending the \c basePath to the previous path this object was already assigned to).

		@note If \c basePath is not an absolute path, this function always returns false and does not alter the path held by this object.

		For example:
		@code{.cpp}
		limes::files::FilesystemEntry path1 { "a/relative/path" };
		limes::files::FilesystemEntry path2 { "/an/absolute/path" };

		const limes::files::Path basePath { "some/other/directory" };

		path1.makeAbsoluteRelativeTo (basePath);
		path2.makeAbsoluteRelativeTo (basePath);

		assert (path1.getAbsolutePath() == "some/other/directory/a/relative/path");
		assert (path2.getAbsolutePath() == "/an/absolute/path");
		@endcode

		@returns True if this function changed the path this object refers to.
		@see makeAbsoluteRelativeToCWD()

		@todo test coverage -- should return false if basePath is not absolute
	 */
	bool makeAbsoluteRelativeTo (const Path& basePath) noexcept;

	/** Similar to \c makeAbsoluteRelativeTo(), but uses the current working directory as the base path.

		@returns True if this function changed the path this object refers to.

		@see makeAbsoluteRelativeTo(), dirs::cwd()
	 */
	bool makeAbsoluteRelativeToCWD() noexcept;

	///@}

	/** @name Path queries */
	///@{

	/** Returns the current path held by this object.
		If this object was constructed with or assigned to a relative path, this function will return that relative path.

		The returned path will be normalized using the normalizePath() function.

		@param makePreferred If true, this converts any directory separators present in the path to the platform's preferred
		directory separator. See \c dirSeparator()

		@see getAbsolutePath()

		@todo test coverage for makePreferred=true
	 */
	[[nodiscard]] Path getPath (bool makePreferred = false) const noexcept;

	/** Returns the full, absolute path of this filesystem entry.

		The returned path will be normalized using the normalizePath() function.

		@param makePreferred If true, this converts any directory separators present in the path to the platform's preferred
		directory separator. See \c dirSeparator()

		@see getPath()

		@todo test coverage for makePreferred=true
	 */
	[[nodiscard]] Path getAbsolutePath (bool makePreferred = false) const noexcept;

	/** Converts this filesystem entry to the path it holds.
		@see getPath()
	 */
	operator Path() const noexcept;

	/** Returns the last section of the path (the filename). */
	[[nodiscard]] std::string getName() const noexcept;

	/** Returns the directory containing this filesystem entry.

		If this FilesystemEntry is a directory, its own path is returned; otherwise, the path of the directory containing
		this file is returned.

		For example:
		@code{.cpp}
		const limes::files::FilesystemEntry path1 { "path/to/a/directory" };
		const limes::files::FilesystemEntry path2 { "path/to/a/file.json" };

		const auto dir1 = path1.getDirectory();
		const auto dir2 = path2.getDirectory();

		assert (dir1.getAbsolutePath() == "path/to/a/directory");
		assert (dir2.getAbsolutePath() == "path/to/a");
		@endcode

		@see getParentDirectory()
	 */
	[[nodiscard]] Directory getDirectory() const;

	/** Returns the parent %directory of this filesystem entry.

		If this FilesystemEntry is a directory, this returns its parent directory; otherwise, this returns the parent directory
		of the directory containing this file or symlink.

		For example:
		@code{.cpp}
		const limes::files::FilesystemEntry path1 { "path/to/a/directory" };
		const limes::files::FilesystemEntry path2 { "path/to/a/file.json" };

		const auto dir1 = path1.getParentDirectory();
		const auto dir2 = path2.getParentDirectory();

		assert (dir1.getAbsolutePath() == "path/to/a");
		assert (dir2.getAbsolutePath() == "path/to");
		@endcode

		@see getDirectory()
	 */
	[[nodiscard]] Directory getParentDirectory() const;

	/** Returns true if this filesystem entry is in a subdirectory of the given \c directory -- ie, if that directory is a parent
		or grandparent of this filepath.

		@param directory The directory that may be a parent of this path.

		@param depthLimit Specifies the number of parent paths of this entry that will be checked for equivalence to \c directory .
		Setting this to 1 will only allow this function to return true if \c directory is the directory this entry is in (its direct
		parent); 2 would return true if \c directory is this path's directory or the directory's parent directory, etc.

		@todo test coverage for false case
	 */
	[[nodiscard]] bool isBelow (const Directory& directory, std::size_t depthLimit = 50) const;

	/** Returns true if the path this object holds is an absolute path.

		Note that this function does not always return true if \c isRelativePath() returns false -- if \c isValid() returns false,
		then both \c isAbsolutePath() and \c isRelativePath() will always return false.

		@see isRelativePath()
	 */
	[[nodiscard]] bool isAbsolutePath() const noexcept;

	/** Returns true if the path this object holds is a relative path.

		Note that this function does not always return true if \c isAbsolutePath() returns false -- if \c isValid() returns false,
		then both \c isAbsolutePath() and \c isRelativePath() will always return false.

		@see isAbsolutePath()
	 */
	[[nodiscard]] bool isRelativePath() const noexcept;

	/** Returns true if this object holds a valid filesystem path.

		@see exists(), isValidPath()
	 */
	[[nodiscard]] bool isValid() const noexcept;

	/** Returns true if this path is a hidden file or directory.
		On Linux and iOS, this returns true if the filename begins with a dot (.) character. On Windows and MacOS, the actual
		OS filesystem APIs are queried.
	 */
	[[nodiscard]] bool isHidden() const;

	/** Returns a filesystem entry with the given name in the same directory as this one. */
	[[nodiscard]] FilesystemEntry getSibling (const std::string_view& siblingName) const;

	///@}

	/** @name Filesystem queries */
	///@{

	/** Returns true if this filesystem entry refers to a %file.
		If this path is not an existing directory or symlink, this class will assume that the path refers to a nonexistent file,
		because a file is assumed to be the "default" type of filesystem object.

		@see isDirectory(), isSymLink()
	 */
	[[nodiscard]] virtual bool isFile() const noexcept;

	/** Returns true if this filesystem entry refers to a %directory.
		@see isFile(), isSymLink()
	 */
	[[nodiscard]] virtual bool isDirectory() const noexcept;

	/** Returns true if this filesystem entry refers to a symbolic link.
		@see isFile(), isDirectory()
	 */
	[[nodiscard]] virtual bool isSymLink() const noexcept;

	/** Returns true if the filesystem object this object refers to exists on disk.
		@see isValid()
	 */
	[[nodiscard]] bool exists() const noexcept;

	/** Returns true if this filesystem object exists.
		@see exists()

		@todo test coverage
	 */
	operator bool() const noexcept;

	/** Returns the size, in bytes, of the filesystem entry referred to by this object.
		If the filesystem entry doesn't exist, this will return 0. For directories, this returns the cumulative size of all the
		directory's contents, recursively. For symbolic links, this returns the size of the link itself, not what it points to.
	 */
	[[nodiscard]] virtual std::uintmax_t sizeInBytes() const;

	/** Returns the last modification time of the filesystem entry.
		@todo test coverage
	 */
	[[nodiscard]] Time getLastModificationTime() const noexcept;

	/** Returns a Volume object representing the logical filesystem volume that this object exists on.
		Returns a \c nullopt if the volume for this path cannot be computed correctly, or if this FilesystemEntry holds an invalid path.
	 */
	[[nodiscard]] std::optional<Volume> getVolume() const noexcept;

	///@}

	/** @name Type conversion */
	///@{

	/** If this object refers to a %file, constructs a File object referring to the same %file. Otherwise, returns a \c nullopt.
		@see getDirectoryObject(), getSymLinkObject()

		@todo test coverage for nullopt case
	 */
	[[nodiscard]] std::optional<File> getFileObject() const noexcept;

	/** If this object refers to a %directory, constructs a Directory object referring to the same %directory. Otherwise, returns a \c nullopt.
		@see getFileObject(), getSymLinkObject()

		@todo test coverage for nullopt case
	 */
	[[nodiscard]] std::optional<Directory> getDirectoryObject() const noexcept;

	/** If this object refers to a symbolic link, constructs a SymLink object referring to the same link. Otherwise, returns a \c nullopt.
		@see getFileObject(), getDirectoryObject()

		@todo better test coverage for function's internal branches
	 */
	[[nodiscard]] std::optional<SymLink> getSymLinkObject() const noexcept;

	///@}

	/** @name Filesystem actions */
	///@{

	/** Creates the filesystem entry this object refers to, if it doesn't already exist.

		@returns True if the filesystem object needed to be created, and was successfully created. False if it already existed
		or if creation fails.

		@todo test coverage for directories and symlinks
	 */
	virtual bool createIfDoesntExist() const noexcept;

	/** Deletes the filesystem entry this object refers to, if it exists.

		@returns True if the object existed and was successfully deleted. False if it did not exist or if deletion fails.

		@see moveToTrash()
	 */
	bool deleteIfExists() const noexcept;

	/** Attempts to move this filesystem object to the system's trash folder.

		@returns True if the object existed and moving to the trash folder was successful.

		@see deleteIfExists()
	 */
	bool moveToTrash() noexcept;

	/** If the filesystem entry this object refers do doesn't exist, this function creates it.
		If it did already exist, this function updates its modification time to the current system time.
		@see touch_noCreate()

		@todo test coverage
	 */
	void touch() const;

	/** The same as \c touch() , except this function does not create the filesystem entry if it didn't already exist.

		@returns True if the filesystem entry existed when this function was called.

		@see touch()
	 */
	bool touch_noCreate() const;

	/** Renames this filesystem object to a new path.
		The filesystem object is moved on disk, and this object stores the new path.

		To manipulate the path this object holds without writing the changes to disk, see \c assignPath() , \c changeName() , etc.

		@returns True if renaming was successful.

		@internal
		This currently always sets the new path as absolute, even if the object was previously holding a relative path.
		@endinternal
	 */
	bool rename (const Path& newPath) noexcept;

	///@}

	/** @name Permissions */
	///@{

	/** Sets the permissions for the filesystem entry.

		@returns True if setting permissions was successful

		@see getPermissions()
	 */
	bool setPermissions (FSPerms permissions, PermOptions options = PermOptions::replace) const noexcept;

	/** Returns the current permissions of the filesystem entry this object refers to.
		@see setPermissions()
	 */
	[[nodiscard]] Permissions getPermissions() const;

	///@}

	/**	@name Creating copies */
	///@{
	/**
		Creates a copy of this filesystem entry at a new location on disk.

		@returns True if copying was successful.
	 */
	bool copyTo (const Path& dest, CopyOptions options = CopyOptions::update_existing) const noexcept;

	/** @copydoc copyTo(const Path&,CopyOptions) */
	bool copyTo (const FilesystemEntry& dest, CopyOptions options = CopyOptions::update_existing) const noexcept;

	/** Creates a copy of this filesystem object, with the same filename, in a different %directory.
		If the \c destDirectory is the same directory that this object is in, this function will make no changes on disk and
		return a \c nullopt .

		@returns A new object holding the path this file was copied to, or a \c nullopt if copying was not successful.
	 */
	std::optional<FilesystemEntry> copyToDirectory (const Path& destDirectory, CopyOptions options = CopyOptions::update_existing) const noexcept;
	///@}

	/** Overwriting */
	//@{
	/** Overwrites this filesystem entry by copying from another location.
		@returns True if copying was successful.
	 */
	bool copyFrom (const Path& source, CopyOptions options = CopyOptions::update_existing) const noexcept;
	bool copyFrom (const FilesystemEntry& source, CopyOptions options = CopyOptions::update_existing) const noexcept;
	//@}

	/** Uses the system's native file browser to display this file to the user.
		This will open a Finder window on Mac, a file explorer window on Windows, etc.

		This function returns immediately.
	 */
	bool revealToUserInFileBrowser() const;

private:
	bool filenameBeginsWithDot() const;

	Path path;
};

/** Writes a FilesystemEntry's path to the output stream.

	@ingroup files
	@relates FilesystemEntry
 */
std::ostream& operator<< (std::ostream& os, const FilesystemEntry& value);

}  // namespace limes::files

namespace std
{

/** A specialization of \c std::hash for filesystem entries.
	The hash value is computed based on the absolute path of the filesystem entry.

	@ingroup files
	@relates FilesystemEntry
 */
template <>
struct LFILE_EXPORT hash<limes::files::FilesystemEntry> final
{
	size_t operator() (const limes::files::FilesystemEntry& e) const noexcept;
};
}  // namespace std
