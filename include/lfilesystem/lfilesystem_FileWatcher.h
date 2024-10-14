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

#include <memory>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"

/** @file
	This file defines the files::FileWatcher class.

	@ingroup limes_files
 */

namespace limes::files
{

/** This class listens for changes to or operations on a certain file,
	and receives callbacks to be notified when changes occur.

	You can create a FileWatcher to listen for events to any kind of
	\c FilesystemEntry .

	The callbacks are asynchronous, and have no guarantee of granularity
	or regularity. Note that the callbacks may be called from a background
	thread!

	@internal
	On MacOS, the \c FSEventStreamCreate() API is used to register a callback
	directly with the OS. On Linux and Windows, Limes creates a background
	thread that polls for changes and calls the listeners when changes have
	occurred. Only one background thread will be created no matter how many
	\c FileWatcher objects there are. Linux uses the \c inotify API and
	Windows uses the \c CreateFileW()/ReadDirectoryChangesW() API.
	@endinternal

	Each of the callbacks receives a \c FilesystemEntry argument with the
	path to which the current event applies. If a directory is being
	watched, this path may be a child of the watched directory. In some cases,
	this path may be a parent directory of the watched path, even if the
	watched path is a file.

	@ingroup limes_files

	@see SimpleFileWatcher

	@todo iOS, Android and Emscripten implementations. This class requires
	platform-specific code, probably JNI for Android and Swift/ObjC for iOS. Is
	this class possible to implement for Emscripten?

	@todo how to test this class? A test would basically need to check that the
	proper callbacks are indeed called when a watched file is mutated. Is this
	even possible?
 */
class LFILE_EXPORT FileWatcher
{
public:
	/** Creates a FileWatcher to watch the given file or directory.

		@throws std::runtime_error Throws an exception if the file watcher
		fails to initialize for any reason. An exception will always be
		thrown if the file you request to watch does not exist at the time
		of the object's construction.
	 */
	explicit FileWatcher (const FilesystemEntry& fileToWatch);

	/** Creates an inactive FileWatcher that does nothing. Call \c start() and provide
		a new path to watch in order to use this object.
	 */
	FileWatcher() noexcept;

	/** Destructor. */
	virtual ~FileWatcher();

	FileWatcher (const FileWatcher&)			= delete;
	FileWatcher& operator= (const FileWatcher&) = delete;

	FileWatcher (FileWatcher&&)			   = delete;
	FileWatcher& operator= (FileWatcher&&) = delete;

	/** Called when a file's content is accessed.
		This is never called on Windows.
	 */
	virtual void fileAccessed (const FilesystemEntry& /*path*/) { }

	/** Called when a file's metadata is changed, such as permissions, owner, etc.
		This is never called on Windows.
		@see fileModified()
	 */
	virtual void fileMetadataChanged (const FilesystemEntry& /*path*/) { }

	/** Called when a file handle is closed (from either reading or writing).
		This is only called on Linux.
	 */
	virtual void fileHandleClosed (const FilesystemEntry& /*path*/) { }

	/** Called when a file or subdirectory is created in a watched directory. */
	virtual void fileCreated (const FilesystemEntry& /*path*/) { }

	/** Called when a file is deleted. */
	virtual void fileDeleted (const FilesystemEntry& /*path*/) { }

	/** Called when a file's content is modified.

		On Windows, there is no distinction between events for a file's content
		being modified and its metadata being modified. This function will always be
		called in both cases, and \c fileMetadataChanged() will never be called on
		Windows.
	 */
	virtual void fileModified (const FilesystemEntry& /*path*/) { }

	/** Called when a file is moved or renamed.
		For directories, this will be called if the watched directory contained the file
		that was moved from or the file that was moved to.
	 */
	virtual void fileMoved (const FilesystemEntry& /*path*/) { }

	/** Called when a file handle is created (for either reading or writing).
		This is only called on Linux.
	 */
	virtual void fileOpened (const FilesystemEntry& /*path*/) { }

	/** Called when an unspecified event has occurred, such as one of the watched file's
		parent directories being renamed or deleted. This is currently only called on Mac
		when the \c kFSEventStreamEventFlagRootChanged event flag is received.
	 */
	virtual void otherEventType (const FilesystemEntry& /*path*/) { }

	/** Restarts watching the path specified at construction.

		@returns True if the FilesystemWatcher is running after this function returns. This
		will return true if the watcher was already running before this function call.
	 */
	bool start();

	/** Begins watching the new specified path.

		If the new path is not the same as the previously watched path, this will first call
		\c stop() to cease monitoring events from the old path, then attempt to start watching
		events for the new path, and return true if this initialization is successful.
	 */
	bool start (const FilesystemEntry& newPathToWatch);

	/** Stops the FilesystemWatcher's event callbacks.
		This does not cancel any pending callbacks that may have been registered with the OS.
	 */
	void stop();

	/** Returns true if the FilesystemWatcher is currently monitoring events.
	 */
	bool isRunning();

	/** Returns the path that is currently being watched. */
	[[nodiscard]] FilesystemEntry getWatchedPath() const noexcept;

	/** Returns true if the current system supports file event watching.
		Currently returns false on iOS and Android.
	 */
	static bool supportedBySystem() noexcept;

private:
	class Impl;

	[[maybe_unused]] std::unique_ptr<Impl> pimpl;

	[[maybe_unused]] FilesystemEntry watchedPath;
};

}  // namespace limes::files
