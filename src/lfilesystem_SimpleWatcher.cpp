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

#include <utility>
#include "lfilesystem/lfilesystem_SimpleWatcher.h"

namespace limes::files
{

SimpleFileWatcher::SimpleFileWatcher (const FilesystemEntry& fileToWatch,
									  Callback&&			 callbackToUse)
	: FileWatcher (fileToWatch), callback (std::move (callbackToUse))
{
	LIMES_ASSERT (callback != nullptr);
}

void SimpleFileWatcher::fileAccessed (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileMetadataChanged (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileHandleClosed (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileCreated (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileDeleted (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileModified (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileMoved (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::fileOpened (const FilesystemEntry& f)
{
	callback (f);
}

void SimpleFileWatcher::otherEventType (const FilesystemEntry& f)
{
	callback (f);
}

}  // namespace files
