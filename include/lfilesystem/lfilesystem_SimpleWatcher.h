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
#include "lfilesystem_Export.h"
#include "lfilesystem_FileWatcher.h"

/** @file
	This file defines the SimpleFileWatcher class.
	@ingroup files
 */

namespace limes::files
{

/** This class implements all of \c FileWatcher's virtual methods, and allows
	you to easily specify a lambda function that will be called for any event
	type.

	@ingroup files

	@see FileWatcher
 */
class LFILE_EXPORT SimpleFileWatcher final : public FileWatcher
{
public:
	/** The type of the lambda that will be called for any event type.

		The argument is the file that was affected by the action, so if a
		directory is being watched, this may be a child path of the watched
		directory.
	 */
	using Callback = std::function<void (const FilesystemEntry&)>;

	/** Creates a file watcher that will call the given \c callback for every
		event type on the watched path.

		@throws std::runtime_error Throws an exception if the file watcher
		fails to initialize for any reason. An exception will always be
		thrown if the file you request to watch does not exist at the time
		of the object's construction.
	 */
	explicit SimpleFileWatcher (const FilesystemEntry& fileToWatch,
								Callback&&			   callbackToUse);

private:
	void fileAccessed (const FilesystemEntry& f) final;
	void fileMetadataChanged (const FilesystemEntry& f) final;
	void fileHandleClosed (const FilesystemEntry& f) final;
	void fileCreated (const FilesystemEntry& f) final;
	void fileDeleted (const FilesystemEntry& f) final;
	void fileModified (const FilesystemEntry& f) final;
	void fileMoved (const FilesystemEntry& f) final;
	void fileOpened (const FilesystemEntry& f) final;
	void otherEventType (const FilesystemEntry& f) final;

	const Callback callback;
};

}  // namespace files
