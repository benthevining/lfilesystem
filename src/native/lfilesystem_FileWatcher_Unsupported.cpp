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

#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FileWatcher.h"

namespace limes::files
{

class LFILE_NO_EXPORT FileWatcher::Impl
{
};

FileWatcher::FileWatcher (const FilesystemEntry&)
{
}

FileWatcher::FileWatcher() noexcept { }

FileWatcher::~FileWatcher() { }

bool FileWatcher::start()
{
	return false;
}

bool FileWatcher::start (const FilesystemEntry&)
{
	return false;
}

void FileWatcher::stop()
{
}

bool FileWatcher::isRunning()
{
	return false;
}

bool FileWatcher::supportedBySystem() noexcept
{
	return false;
}

FilesystemEntry FileWatcher::getWatchedPath() const noexcept
{
	return watchedPath;
}

}  // namespace files
