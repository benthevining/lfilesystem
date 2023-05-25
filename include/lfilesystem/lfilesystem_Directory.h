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

#include <functional>  
#include <cstdint>	  
#include <string>	   
#include <vector>	   
#include <memory>
#include <iterator>
#include "lfilesystem_Export.h"
#include "lfilesystem_FilesystemEntry.h"
#include "lfilesystem_File.h"				
#include "lfilesystem_Sym_Link.h"			

/** @file
	This file defines the Directory class.

	@ingroup files
 */

namespace limes::files
{

/** This class represents a %directory on the filesystem.

	This class provides a high-level API for treating directories almost like containers,
	with methods for iterating over its children.

	For example, this code prints all filenames present in a directory and all its subdirectories:
	@code{.cpp}
	const limes::files::Directory dir { "/my/directory" };

	dir.iterateFiles ([](const limes::files::File& f){ std::cout << f.getFilename() });
	@endcode

	@test This class is covered by unit tests.

	@ingroup files

	@todo getNonexistentChild()
 */
class LFILE_EXPORT Directory final : public FilesystemEntry
{
public:
	using FilesystemEntry::FilesystemEntry;

	Directory (const Directory&) = default;
	Directory& operator= (const Directory&) = default;

	Directory (Directory&&) = default;
	Directory& operator= (Directory&&) = default;

	/** @name Typedefs
		Typedefs for callbacks used by the functions that iterate through this directory's children.
	 */
	///@{

	/** A callback for File objects. */
	using FileCallback = std::function<void (const File&)>;

	/** A callback for Directory objects. */
	using DirectoryCallback = std::function<void (const Directory&)>;

	/** A callback for SymLink objects. */
	using SymLinkCallback = std::function<void (const SymLink&)>;

	/** A callback for FilesystemEntry objects. */
	using FilesystemEntryCallback = std::function<void (const FilesystemEntry&)>;

	///@}

	/** @name Assignment */
	///@{
	/** Assigns this object to refer to a new path.
		@todo test coverage
	 */
	Directory& operator= (const Path& newPath);
	Directory& operator= (const std::string_view& newPath);
	///@}

	/** @name Filesystem queries */
	///@{

	/** Returns true if this %directory contains the passed FilesystemEntry.

		@param depthLimit Specifies the number of parent paths of the \c entry that will be checked for equivalence
		to this directory . Setting this to 1 will only allow this function to return true if this is the immediate
		parent directory of \c entry ; 2 would return true this is the entry's directory or that directory's
		parent directory, etc.

		@see FilesystemEntry::isBelow()
	 */
	[[nodiscard]] bool contains (const FilesystemEntry& entry, std::size_t depthLimit = 50) const;

	/** Returns true if this %directory contains a child with the specified name.

		@note This function is not recursive; this only searches this directory's immediate children for a
		matching filename.
	 */
	[[nodiscard]] bool contains (const std::string_view& childName) const;

	/** Returns true if this %directory contains no children. */
	[[nodiscard]] bool isEmpty() const;

	/** Returns the size of this %directory, calculated as the cumulative size of all of this
		directory's contents, including all subdirectories recursively.
	 */
	[[nodiscard]] std::uintmax_t sizeInBytes() const final;

	[[nodiscard]] bool isDirectory() const noexcept final;
	[[nodiscard]] bool isFile() const noexcept final;
	[[nodiscard]] bool isSymLink() const noexcept final;

	///@}

	/** Creates the %directory this object refers to, if it doesn't already exist.

		@returns True if the %directory needed to be created, and was successfully created. False if the
		directory already existed.
	 */
	bool createIfDoesntExist() const noexcept final;

	/** Returns the input path made relative to this directory's path.

		For example:
		@code{.cpp}
		const limes::files::Directory dir { "/path/to/a/directory" };

		const auto path = dir.getRelativePath ("/path/to/a/directory/with/nested/subdirs/and/filename.txt");

		assert (path == "with/nested/subdirs/and/filename.txt");
		@endcode
	 */
	[[nodiscard]] Path getRelativePath (const Path& inputPath) const;

	/** @name Child files */
	///@{

	/** Returns a File object representing a file in this %directory with the specified name.
		@see getChild()
	 */
	[[nodiscard]] File getChildFile (const std::string_view& filename, bool createIfNeeded = false) const;

	/** Returns all child files that exist in this %directory.
		@see iterateFiles()
	 */
	[[nodiscard]] std::vector<File> getChildFiles (bool recurse = true, bool includeHiddenFiles = true) const;

	/** Calls a function for each child %file that exists in this %directory.
		@see getChildFiles()
		@todo test coverage
	 */
	void iterateFiles (FileCallback&& callback, bool recurse = true, bool includeHiddenFiles = true) const;

	///@}

	/** @name Child subdirectories */
	///@{

	/** Returns true if this directory contains at least one subdirectory. */
	[[nodiscard]] bool containsSubdirectories() const;

	/** Returns a Directory object that represents a subdirectory of this one with the specified name.
		@see getChild()
	 */
	[[nodiscard]] Directory getChildDirectory (const std::string_view& subdirectoryName, bool createIfNeeded = false) const;

	/** Returns all child directories that exist in this %directory.
		@see iterateDirectories()
	 */
	[[nodiscard]] std::vector<Directory> getChildDirectories (bool recurse = true, bool includeHiddenFiles = true) const;

	/** Calls a function for each child %directory that exists in this %directory.
		@see getChildDirectories()
		@todo test coverage
	 */
	void iterateDirectories (DirectoryCallback&& callback, bool recurse = true, bool includeHiddenFiles = true) const;

	///@}

	/** Child symbolic links */
	///@{

	/** Returns a SymLink object that represents a symbolic link in the current %directory with the specified name.
		Note that this function always creates the symbolic link if it doesn't exist.

		@throws std::runtime_error An exception will be thrown if creating the symbolic link fails.

		@see getChild()
	 */
	[[nodiscard]] SymLink createChildSymLink (const std::string_view& symLinkName,
											  const FilesystemEntry&  symLinkTarget) const;

	/** Returns all child symbolic links that exist in this %directory.
		@see iterateSymLinks()
	 */
	[[nodiscard]] std::vector<SymLink> getChildSymLinks (bool recurse = true, bool includeHiddenFiles = true) const;

	/** Calls a function for each child symbolic link that exists in this %directory.
		@see getChildSymLinks()
		@todo test coverage
	 */
	void iterateSymLinks (SymLinkCallback&& callback, bool recurse = true, bool includeHiddenFiles = true) const;

	///@}

	/** @name Other children functions */
	///@{

	/** Returns a type-erased FilesystemEntry representing a filesystem object in this %directory with the specified name.
		@see getChildFile(), getChildDirectory(), createChildSymLink(), getAllChildren()
	 */
	[[nodiscard]] FilesystemEntry getChild (const std::string_view& childName, bool createIfNeeded = false) const;

	/** Returns all child filesystem entries that exist in this %directory.
		@see iterateAllChildren(), getChild()
	 */
	[[nodiscard]] std::vector<FilesystemEntry> getAllChildren (bool recurse = true, bool includeHiddenFiles = true) const;

	/** Iterates through all child objects of this %directory, calling different callbacks for each object depending on
		if it is a %file, %directory, or symbolic link. It is not an error for any of these callbacks to be \c nullptr .
		@see getAllChildren()
		@todo test coverage
	 */
	void iterateAllChildren (FileCallback&&		 fileCallback,
							 DirectoryCallback&& directoryCallback,
							 SymLinkCallback&&	 symLinkCallback,
							 bool				 recurse			= true,
							 bool				 includeHiddenFiles = true) const;

	/** Iterates through all child objects of this %directory, calling a single type-erased callback for each one.
		@see getAllChildren()
		@todo test coverage
	 */
	void iterateAllChildren (FilesystemEntryCallback&& callback,
							 bool					   recurse			  = true,
							 bool					   includeHiddenFiles = true) const;

	///@}

	/** An iterator class that allows iterating a directory like a standard C++ container.
		The directory class will create an array of all its children when you call \c begin() ;
		the iterator will advance through this array as you increment it.

		@see begin(), end()
	 */
	struct LFILE_EXPORT Iterator final
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type		= FilesystemEntry;
		using difference_type	= std::ptrdiff_t;
		using pointer			= FilesystemEntry*;
		using reference			= FilesystemEntry&;

		Iterator&			   operator++();
		[[nodiscard]] Iterator operator++ (int);
		bool				   operator== (Iterator other);
		bool				   operator!= (Iterator other);

		reference operator*() const;
		pointer	  operator->() const;

		explicit Iterator();

		Iterator (const Iterator&) = default;
		Iterator& operator= (const Iterator&) = default;

		Iterator (Iterator&&) = default;
		Iterator& operator= (Iterator&&) = default;

	private:
		using VectorType = std::vector<FilesystemEntry>;

		explicit Iterator (VectorType&& v);

		std::shared_ptr<VectorType> entries { nullptr };

		VectorType::size_type idx { 0UL };

		friend class Directory;
	};

	/** Returns an iterator to the first entry in this directory. */
	[[nodiscard]] Iterator begin() const;

	/** Returns an iterator to the last entry in this directory. */
	[[nodiscard]] Iterator end() const;

	/** @name The current working %directory */
	///@{

	/** Sets this %directory as the current working %directory.
		@see isCurrentWorkingDirectory(), dirs::setCWD()
		@todo test coverage
	 */
	bool setAsWorkingDirectory() const;

	/** Returns true if this directory is the system's current working directory.

		@see setAsWorkingDirectory(), dirs::cwd()
	 */
	[[nodiscard]] bool isCurrentWorkingDirectory() const;

	///@}
};

}  // namespace limes::files
