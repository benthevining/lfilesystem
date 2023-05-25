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

#include <cstdio>
#include "lfilesystem_Export.h"
#include "lfilesystem_FilesystemEntry.h"

/** @file
	This file defines the CFile class.

	@ingroup files
 */

namespace limes::files
{

class File;

/** This class is a wrapper around a C-style \c FILE* that takes care of freeing the file when the object
	is destroyed, and provides a few other convenience methods.

	This class is intended as a way to allow interfacing with C APIs that require a \c FILE* while maintaining
	RAII-style resource management.

	For example:
	@code{.cpp}
	const limes::files::File myFile { "/path/to/some/file" };

	const auto cFile = myFile.getCfile();

	call_some_c_api (cFile.get());
	@endcode

	@ingroup files
	@see File
 */
class LFILE_EXPORT CFile final
{
public:
	/** Represents possible modes a file can be opened in. */
	enum class Mode
	{
		Read,			///< Reads the file from the start.
		Write,			///< Write to the file. Creates a new file if this path didn't exist; overwrites the file's contents if it did exist.
		Append,			///< Append to the file. A new file will be created if this path did not exist.
		ReadExtended,	///< The same as Read, but in extended mode.
		WriteExtended,	///< The same as Write, but in extended mode.
		AppendExtended	///< The same as Append, but in extended mode.
	};

	/** @name Constructors */
	///@{

	/** Creates a default CFile that holds a nullptr. Call \c open() to actually open a file. */
	CFile() = default;

	/** Creates a CFile that takes ownership of the passed \c FILE* .

		@note The CFile object will take ownership of the passed pointer, so you should not delete it manually!
	 */
	explicit CFile (std::FILE* fileHandle) noexcept;

	/** Creates a CFile referencing the given filepath, in the given mode.
		\c fopen() is called to open the file.
		@see open()
	 */
	explicit CFile (const Path& filepath, Mode mode) noexcept;

	/** Move constructor. */
	CFile (CFile&& other) noexcept;

	///@}

	/** Destructor. If the file is open when the destructor is called, the destructor will close the file.
		@see close()
	 */
	~CFile() noexcept;

	/** Move assignment operator. */
	CFile& operator= (CFile&& other) noexcept;

	CFile (const CFile&) = delete;
	CFile& operator=(const CFile&) = delete;

	/** @name Accessors */
	///@{
	/** Returns the pointer this object holds.
		Note that the CFile object still retains ownership of this pointer, you should not free it!
	 */
	[[nodiscard]] std::FILE* get() const noexcept;

	/** Returns the pointer this object holds. */
	std::FILE* operator->() const noexcept;

	/** Dereferences the pointer this object holds.
		Note that the pointer may be null, so be careful!
	 */
	std::FILE& operator*() const;

	/** Returns the path of the file this object represents.
		Returns an empty path if no file is currently open.
	 */
	[[nodiscard]] Path getPath() const;

	/** Returns the pointer held by this object. */
	operator std::FILE*() const noexcept;
	///@}

	/** If the file is currently open, this closes it by calling \c fclose() .

		@post Calling \c get() after calling this function returns a \c nullptr and calling \c getPath()
		after calling this function returns an empty path.
	 */
	void close() noexcept;

	/** Closes the current file (if one is open) and opens the file at the specified path.
		\c fopen() is called to open the file.

		@return True if opening the file was successful
	 */
	bool open (const Path& filepath, Mode mode) noexcept;

	/** Returns true if the file is currently open. */
	[[nodiscard]] bool isOpen() const noexcept;

	/** Evaluates to true if the file is currently open. */
	explicit operator bool() const noexcept;

	/** Returns a File object representing this %file. Note that if \c getPath() returns an empty path,
		then this function will return an invalid File object. The returned File object will also be invalid if no
		file handle was open when you called this function.
	 */
	[[nodiscard]] File getFile() const;

	/** Creates an automatically named, self-deleting temporary file using the C function \c std::tmpfile .

		@see TempFile

		@internal
		Can this be removed? Perhaps just TempFile::getNextFile() would suffice?
		@endinternal
	 */
	[[nodiscard]] static CFile createTempFile();

private:
	std::FILE* ptr { nullptr };
};

}  // namespace limes::files
