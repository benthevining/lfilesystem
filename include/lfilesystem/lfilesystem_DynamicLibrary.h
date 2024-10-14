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

#if defined(_WIN32) || defined(WIN32)
#	include <windows.h>
#endif

#include <string_view>
#include <string>
#include <atomic>
#include <vector>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FileWatcher.h"
#include "lfilesystem/lfilesystem_File.h"

/** @file
	This file defines the DynamicLibrary class.

	@ingroup limes_files
 */

namespace limes::files
{

/** This class represents a dynamically loaded library.
	On Unixes, this is the equivalent of using \c dlopen() , but wrapped in an RAII class that
	frees the library handle when it is destroyed.

	This class can also locate the file on disk that the library was loaded from, and "reload" itself.

	All functions of this class are thread-safe; the library handle is stored atomically.

	@ingroup limes_files

	@todo double check iOS. I've heard that iOS disallows calling \c dlopen() .

	@todo use emscripten_dlopen when building with Emscripten. Note that this function is
	asynchronous, however.

	@todo function to add a directory to the search paths for DLLs?

	@todo function to check if dynamic libraries are supported by the current system
 */
class LFILE_EXPORT DynamicLibrary final
{
public:
	/** @name Constructors */
	///@{

	/** Creates an unopened library object. Call \c open() to actually open a library. */
	DynamicLibrary() = default;

	/** Creates a library object and attempts to open the specified library.
		@see open()
	 */
	explicit DynamicLibrary (const std::string_view& nameOrPath) noexcept;

	///@}

	/** Destructor. If the library is open when this object is destroyed, the destructor will close the library.
		@see close()
	 */
	~DynamicLibrary();

	DynamicLibrary (DynamicLibrary&& other) noexcept;
	DynamicLibrary& operator= (DynamicLibrary&& other) noexcept;

	DynamicLibrary (const DynamicLibrary&)			  = delete;
	DynamicLibrary& operator= (const DynamicLibrary&) = delete;

#if (defined(_WIN32) || defined(WIN32)) && ! defined(DOXYGEN)
	using Handle = HMODULE;
#else
	using Handle = void*;
#endif

	/** @typedef Handle
		The type of platform-specific handle used for dynamic libraries.
		This will be \c HMODULE on Windows and \c void* everywhere else.
		This will always be a pointer or pointer-like type.
	 */

	/** Returns true if this library object refers to the same shared library as the other one. */
	[[nodiscard]] bool operator== (const DynamicLibrary& other) const noexcept;

	/** Returns true if this library object does not refer to the same shared library as the other one. */
	[[nodiscard]] bool operator!= (const DynamicLibrary& other) const noexcept;

	/** Returns true if the library is currently open.
		This can be called after the constructor, or after calling open(), to check if the library opened correctly.
	 */
	[[nodiscard]] bool isOpen() const noexcept;

	/** Returns a platform-specific handle to the currently open shared library (if any). */
	[[nodiscard]] Handle getHandle() const noexcept;

	/** Attempts to locate a function within the shared library, and, if found, returns a pointer to the function. */
	[[nodiscard]] void* findFunction (const std::string_view& functionName) noexcept;

	/** Attempts to open a new shared library.
		If one was previously open, calling this will close the old library (even if the new library fails to open).

		This will call \c libraryOpened() on all listeners.

		@return True if the library was opened successfully

		@see Listener::libraryOpened()
	 */
	bool open (const std::string_view& nameOrPath) noexcept;

	/** Closes the shared library, if one is open.

		This will call \c libraryClosed() on all listeners.

		@post Calling \c getHandle() after calling this function will return \c nullptr .

		@see Listener::libraryClosed()
	 */
	void close();

	/** Closes and reloads the library.
		This locates the file on disk from which the library was loaded, then closes the library and reopens it
		from this same file.

		@returns True if the library was reloaded successfully; false if the library was not open when this was
		called, or if reopening it fails.

		@see Reloader
	 */
	bool reload();

	/** Attempts to locate the file on disk where the code for the current shared library is actually located.
		If the library isn't open, returns a null file.

		@todo Emscripten implementation
	 */
	[[nodiscard]] File getFile() const;

	/** Attempts to determine the name of the library, if it is open. If the library isn't open, returns an empty string. */
	[[nodiscard]] std::string getName() const;

	/** This class watches a dynamic library file for changes on disk.
		If the file is deleted, this calls \c DynamicLibrary::close() , and if the file is modified, this calls
		\c DynamicLibrary::reload() .

		@see DynamicLibrary::reload()
	 */
	class LFILE_EXPORT Reloader final : public FileWatcher
	{
	public:
		/** Creates a Reloader object watching the specified library. */
		explicit Reloader (DynamicLibrary& libraryToReload);

	private:
		void fileDeleted (const FilesystemEntry&) final;
		void fileModified (const FilesystemEntry&) final;

		DynamicLibrary& library;
	};

	/** This class listens for events to a DynamicLibrary and receives a callback
		when the library is opened, closed, or reloaded.

		Note that all callbacks may be called from any thread! You need to ensure
		thread safety in your callbacks.
	 */
	class LFILE_EXPORT Listener
	{
	public:
		/** Constructs a listener. */
		explicit Listener (DynamicLibrary& library);

		/** Destructor. */
		virtual ~Listener();

		/** Called when the library is opened. Note that if DynamicLibrary::reload()
			is called, then listeners will receive a \c libraryReloaded() call instead
			of \c libraryClosed() and \c libraryOpened() .

			@see DynamicLibrary::open(), libraryReloaded()
		 */
		virtual void libraryOpened (bool /*wasSuccessful*/) { }

		/** Called when the library is closed. Note that if DynamicLibrary::reload()
			is called, then listeners will receive a \c libraryReloaded() call instead
			of \c libraryClosed() and \c libraryOpened() .

			@see DynamicLibrary::close(), libraryReloaded()
		 */
		virtual void libraryClosed() { }

		/** Called when DynamicLibrary::reload() is called.

			Note that on reload events, only this callback will be fired, and not
			\c libraryClosed() or \c libraryOpened() .

			@see DynamicLibrary::reload()
		 */
		virtual void libraryReloaded (bool /*wasSuccessful*/) { }

	private:
		DynamicLibrary& lib;
	};

private:
	std::atomic<Handle> handle { nullptr };

	std::vector<Listener*> listeners;

	bool suppressNotifs { false };
};

}  // namespace limes::files

namespace std
{

/** A specialization of \c std::hash for dynamic libraries.
	The hash value is computed based on the path of the file containing the dynamic library code.

	@ingroup limes_files
 */
template <>
struct LFILE_EXPORT hash<limes::files::DynamicLibrary> final
{
	size_t operator() (const limes::files::DynamicLibrary& l) const noexcept;
};

}  // namespace std
