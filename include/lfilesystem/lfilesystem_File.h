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

#include <cstddef>	// for size_t
#include <string>	// for string
#include <string_view>
#include <vector>	   // for vector
#include <functional>  // for std::hash
#include <optional>
#include <fstream>
#include <memory>
#include <iterator>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"  // for FilesystemEntry, Path
#include "lfilesystem/lfilesystem_CFile.h"

/** @file
	This file defines the File and TempFile classes.

	@ingroup limes_files
 */

namespace limes::files
{

#pragma mark File
#pragma region File

/** This class represents a %file on the filesystem.
	This class provides a high-level API for reading from and writing to files.

	For example:
	@code{.cpp}
	const limes::files::File file { "/my/file.txt" };

	for (const auto& line : file.loadAsLines())
		std::cout << line << std::endl;

	file.overwrite ("New file content...");
	@endcode

	@ingroup limes_files
	@see CFile

	@todo start as process
	@todo get user/group ID of owner?
 */
class LFILE_EXPORT File : public FilesystemEntry
{
public:
	using FilesystemEntry::FilesystemEntry;

	File (const File&)			  = default;
	File& operator= (const File&) = default;

	File (File&&)			 = default;
	File& operator= (File&&) = default;

	/** @name Path queries */
	///@{

	/** Returns this file's filename.

		@param includeExtension When true, the %file extension is included in the returned string. When false,
		the file extension and trailing dot are omitted.

		@see getFileExtension()
	 */
	[[nodiscard]] std::string getFilename (bool includeExtension = false) const;

	/** Returns this file's %file extension, if any.
		@see getFilename()
	 */
	[[nodiscard]] std::string getFileExtension() const;

	/** Returns true if this %file has the specified %file extension.
		The passed file extension may contain or omit the dot; \c '.json' or \c 'json' will both give the same results.
	 */
	[[nodiscard]] bool hasFileExtension (const std::string_view& extension) const;

	/** Returns true if this %file has a file extension. */
	[[nodiscard]] bool hasFileExtension() const;

	/** Returns true if this file is a MacOS bundle.
		Obviously, this always returns false on non-Apple platforms.
		Currently, this also always returns false on iOS, since there doesn't seem to be a way to detect this
		without trying to open the bundle.
	 */
	[[nodiscard]] bool isMacOSBundle() const noexcept;

	[[nodiscard]] bool isFile() const noexcept final;
	[[nodiscard]] bool isDirectory() const noexcept final;
	[[nodiscard]] bool isSymLink() const noexcept final;

	///@}

	/** @name Path manipulation */
	///@{

	/** Replaces the file extension of this file.
		This function can rename the file on disk, or only alter the internal path held by this object.

		@param newFileExtension The new %file extension to use.

		@param renameOnDisk If true, the %file is actually renamed on the filesystem. If false, this function
		only changes the internal representation of the path, and will always return \c false .

		@returns True if the file was actually renamed on disk successfully. If the \c renameOnDisk parameter
		is false, this function always returns false.

		@see FilesystemEntry::rename()

		@todo test coverage
	 */
	bool replaceFileExtension (const std::string_view& newFileExtension,
							   bool					   renameOnDisk = true);

	/** Assigns this object to refer to a new path.
		@todo test coverage
	 */
	File& operator= (const Path& newPath);

	/** Assigns this object to refer to a new path.
		@todo test coverage
	 */
	File& operator= (const std::string_view& newPath);

	///@}

	/** @name Loading the file */
	///@{

	/** Loads the file's contents as a string.
		@see loadAsLines()
	 */
	[[nodiscard]] std::string loadAsString() const noexcept;

	/** Loads the file's contents as a vector of strings, with each string containing the contents of one line of the %file.

		@see loadAsString()
	 */
	[[nodiscard]] std::vector<std::string> loadAsLines() const;

	/** Returns a standard input stream for reading from this file.
		@see getOutputStream()
		@todo test coverage
	 */
	[[nodiscard]] std::unique_ptr<std::ifstream> getInputStream() const;

	///@}

	/** @name Overwriting with content */
	///@{
	/** Replaces the file's contents with the given data.
		@returns True if writing the data was successful
	 */
	bool overwrite (const char* const data, std::size_t numBytes) const noexcept;
	bool overwrite (const std::string_view& text) const noexcept;

	/** Returns a standard output stream for writing to this file.
		@see getInputStream()
		@todo test coverage
	 */
	[[nodiscard]] std::unique_ptr<std::ofstream> getOutputStream() const;

	///@}

	/** @name Appending content */
	///@{
	/** Appends the given data to the file's current contents.

		For best performance, if you need to append multiple pieces of data to a file, you should prefer to load its previous
		content, make all your alterations, and use one of the \c overwrite() methods. Calling this function repeatedly is
		unnecessarily expensive.

		@returns True if writing the data was successful
	 */
	bool append (const char* const data, std::size_t numBytes) const noexcept;
	bool append (const std::string_view& text) const noexcept;
	///@}

	/** @name Prepending content */
	///@{
	/** Prepends the given data to the file's current contents.
		@returns True if writing the data was successful
	 */
	bool prepend (const char* const data, std::size_t numBytes) const noexcept;
	bool prepend (const std::string_view& text) const noexcept;
	///@}

	/** Resizes this file to the set number of bytes.

		If the file was larger than this size, the extra data is discarded. If the file was smaller, the size
		will be increased via zero-padding. On systems that support sparse files, the allocation won't actually
		take place until non-zero bytes are written to this file, though the filesystem will still store the file's
		new size as the newly increased size.

		The \c allowTruncation and \c allowIncreasing parameters can be used to make sure this function only increases
		or decreases the file's size, though by default it may do either. If both \c allowTruncation and \c allowIncreasing
		are false, an assertion will be thrown, as this doesn't make much sense.

		@returns False if this file didn't already exist, if the passed size was the same as the file's original
		size, or if an error occurs in resizing.
	 */
	bool resize (std::uintmax_t newSizeInBytes, bool allowTruncation = true, bool allowIncreasing = true) const noexcept;

	/** @name Hard links */
	///@{

	/** Creates a hard link to this file at the specified \c path .

		The file at the new \c path will reference the same underlying filesystem object as this file; the
		two will be indistinguishable and it is not possible to tell which was the "original" file.

		If a relative path is passed, it is evaluated relative to the current working directory.

		@returns A File object representing the path of the newly creating hardlink. A \c nullopt may be returned if creating
		the hardlink fails.

		@see getHardLinkCount()
	 */
	std::optional<File> createHardLink (const Path& path) const;

	/** Returns the number of hard links referring to this underlying filesystem object.

		If this file doesn't exist or an error occurs, this will return 0.

		@see createHardLink()
	 */
	[[nodiscard]] std::uintmax_t getHardLinkCount() const noexcept;

	///@}

	/** Duplicates this file within its directory.

		This creates a new file containing the same content as this file, in this file's directory, and automatically names the new file.
		The new file will be named \c \<thisFileName\>_copy.\<thisFileXtn\> , unless a file with that name already exists, in which case
		it will be named \c \<thisFileName\>_copy2.\<thisFileXtn\> , and so on, until a unique name in the current directory is reached.
		If no unique filename can be created using this pattern within the first 999 iterations, then a \c nullopt is returned.

		For example:
		@code{.cpp}
		const limes::files::File file { "/path/to/file.txt" };

		const auto copy1 = file.duplicate();

		// if 'path/to/file_copy.txt' did not exist:
		assert (copy1->getAbsolutePath() == "path/to/file_copy.txt");

		// but if it did exist:
		assert (copy1->getAbsolutePath() == "path/to/file_copy2.txt");

		// and if that path existed:
		assert (copy1->getAbsolutePath() == "path/to/file_copy3.txt");

		// etc...
		@endcode

		@returns The new file that was created, or a \c nullopt if duplication fails.

		@note This always returns \c nullopt if the file does not exist when this function is called.

		@todo test coverage
	 */
	std::optional<File> duplicate() const noexcept;

	/** Returns a CFile referring to this filepath.
		When the CFile constructor is called, a %file handle for this %file will be opened. This will return an invalid CFile object
		if the %file does not exist.
		@see CFile
	 */
	[[nodiscard]] CFile getCfile (CFile::Mode mode = CFile::Mode::Read) const noexcept;

	/** An iterator class that allows iterating a file like a standard C++ container.
		The iterator will advance through the file line-by-line.
		The file class will create an array of all its lines when you call \c begin() ;
		the iterator will advance through this array as you increment it.

		@see begin(), end()
	 */
	struct LFILE_EXPORT Iterator final
	{
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type		= std::string;
		using difference_type	= std::ptrdiff_t;
		using pointer			= std::string*;
		using reference			= std::string&;

		Iterator&			   operator++();
		[[nodiscard]] Iterator operator++ (int);
		bool				   operator== (Iterator other);
		bool				   operator!= (Iterator other);

		reference operator*() const;
		pointer	  operator->() const;

		explicit Iterator();

		Iterator (const Iterator&)			  = default;
		Iterator& operator= (const Iterator&) = default;

		Iterator (Iterator&&)			 = default;
		Iterator& operator= (Iterator&&) = default;

	private:
		using VectorType = std::vector<std::string>;

		explicit Iterator (VectorType&& v);

		std::shared_ptr<VectorType> lines { nullptr };

		VectorType::size_type idx { 0UL };

		friend class File;
	};

	/** Returns an iterator to the first line in this file. */
	[[nodiscard]] Iterator begin() const;

	/** Returns an iterator to the last line in this file. */
	[[nodiscard]] Iterator end() const;

	/** Returns a file representing the location of the executable %file that launched the current process.
		If the calling code is an audio plugin, this will return the path to the DAW it's being run in.

		@note The %file returned by this function may not exist anymore; on some Unixes, it is perfectly
		legal to delete an executable %file while the executable is still running.

		@see getCurrentModule()
	 */
	[[nodiscard]] static File getCurrentExecutable();

	/** Returns a %file representing the location of the current code module.

		If the current process is a dynamic library, this will return the path to the library %file. Otherwise, this may return the same
		thing as \c getCurrentExecutable() .

		If the calling code is an audio plugin, this will return the path to the plugin binary.

		@see getCurrentExecutable()
	 */
	[[nodiscard]] static File getCurrentModule();

private:
	[[nodiscard]] bool write_data (const char* const data, std::size_t numBytes, bool overwrite) const noexcept;
};

/*-------------------------------------------------------------------------------------------------------------------------*/

#pragma mark TempFile
#pragma endregion
#pragma region TempFile

/** Represents a temporary %file.

	This object will create the temporary %file when the object is constructed, and by default will destroy the %file when the object
	is destroyed (though this behavior can be turned off).

	The temporary %file will reside in the %directory returned by \c dirs::temp() .

	If you don't care about filenames and just need a temporary %file to work with, use the static method \c getNextFile() .

	@ingroup limes_files
	@see dirs::temp(), CFile::createTempFile()

	@todo unit tests
 */
class LFILE_EXPORT TempFile final : public File
{
public:
	/** Creates a temporary %file with the specified path.
		The %file will be created on the filesystem when this object is constructed.

		@param filepath The path of the temporary %file. If this is a relative path, then the directory returned by
		\c dirs::temp() will be prepended to the path.

		@param destroyOnDelete If true, the %file will be deleted from the filesystem when this object is destroyed. If false,
		the file will be left on disk when this object is deleted.

		@see getNextFile()
	 */
	explicit TempFile (const Path& filepath,
					   bool		   destroyOnDelete = true);

	/** The temporary %file will be deleted from the filesystem when this object is destroyed, if this object was constructed with
		the \c destroyOnDelete parameter being \c true .
	 */
	~TempFile() final;

	TempFile (const TempFile&)			  = delete;
	TempFile& operator= (const TempFile&) = delete;

	/** @name Moving */
	///@{
	/** Move constructor. */
	TempFile (TempFile&& other) noexcept;

	/** Move assignment operator. */
	TempFile& operator= (TempFile&& other) noexcept;
	///@}

	/** Returns a new temporary %file.
		This will create an unused filename in the directory returned by \c dirs::temp() .
		The TempFile returned by this function will delete the file it references on disk when it is destroyed.
	 */
	[[nodiscard]] static TempFile getNextFile();

private:
	bool shouldDelete { true };
};

/** Writes the file's contents to the output stream.

	@ingroup limes_files
	@relates File
 */
std::ostream& operator<< (std::ostream& os, const File& file);

/** Reads content from the input stream, and overwrites the file with it.

	@ingroup limes_files
	@relates File
 */
std::istream& operator>> (std::istream& is, const File& file);

}  // namespace limes::files

namespace std
{

/** A specialization of \c std::hash for File objects.
	The hash value is computed using the file's contents.

	@ingroup limes_files
 */
template <>
struct LFILE_EXPORT hash<limes::files::File> final
{
	size_t operator() (const limes::files::File& f) const noexcept;
};
}  // namespace std
