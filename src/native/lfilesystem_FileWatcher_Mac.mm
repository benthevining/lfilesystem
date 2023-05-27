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

#import <Foundation/Foundation.h>
#include <stdexcept>
#include <sstream>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FileWatcher.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"

namespace limes::files
{

class LFILE_NO_EXPORT FileWatcher::Impl final
{
public:
	Impl (FileWatcher& parent, const FilesystemEntry& fileToWatch)
		: watcher (parent), watchedPath (fileToWatch.getAbsolutePath())
	{
		NSString* newPath = [NSString stringWithUTF8String:fileToWatch.getAbsolutePath().c_str()];	// cppcheck-suppress syntaxError

		paths					= [[NSArray arrayWithObject:newPath] retain];
		context.version			= 0L;
		context.info			= this;	 // send a pointer to this Impl object as the client callback info
		context.retain			= nil;
		context.release			= nil;
		context.copyDescription = nil;

		stream = FSEventStreamCreate (kCFAllocatorDefault, callback, &context, (CFArrayRef) paths, kFSEventStreamEventIdSinceNow, 0.05,
									  kFSEventStreamCreateFlagNoDefer | kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagWatchRoot);

		if (stream)
		{
			FSEventStreamScheduleWithRunLoop (stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			FSEventStreamStart (stream);
		}
		else
		{
			throw std::runtime_error { "FileWatcher failed to initialize" };
		}
	}

	~Impl()
	{
		if (stream)
		{
			FSEventStreamStop (stream);
			FSEventStreamUnscheduleFromRunLoop (stream, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
			FSEventStreamInvalidate (stream);
			FSEventStreamRelease (stream);
		}
	}

	static void callback (ConstFSEventStreamRef,
						  void*	 clientCallBackInfo,  // a pointer to the Impl object this callback is meant for
						  size_t numEvents, void* eventPaths,
						  const FSEventStreamEventFlags* eventFlags,
						  const FSEventStreamEventId*)
	{
		auto* impl = reinterpret_cast<Impl*> (clientCallBackInfo);

		auto** files = reinterpret_cast<char**> (eventPaths);

		if (files == nullptr || impl == nullptr)
			return;

		for (auto i = 0; i < static_cast<int> (numEvents); ++i)
			impl->handleEvent (files[i], eventFlags[i]);
	}

private:
	void handleEvent (const Path& path, FSEventStreamEventFlags flags) const
	{
		FilesystemEntry file { path };

		file.makeAbsoluteRelativeTo (watchedPath);

		if (flags & kFSEventStreamEventFlagItemModified)
		{
			watcher.fileModified (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagItemRenamed)
		{
			watcher.fileMoved (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagItemCreated)
		{
			watcher.fileCreated (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagRootChanged)
		{
			watcher.otherEventType (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagItemCloned)
		{
			watcher.fileAccessed (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagItemChangeOwner || flags & kFSEventStreamEventFlagItemXattrMod)
		{
			watcher.fileMetadataChanged (file);
			return;
		}

		if (flags & kFSEventStreamEventFlagItemRemoved)
			watcher.fileDeleted (file);
	}

	FileWatcher& watcher;

	Path watchedPath;

	NSArray*					paths;
	FSEventStreamRef			stream;
	struct FSEventStreamContext context;
};

/*---------------------------------------------------------------------------------------------------------------------*/

FileWatcher::FileWatcher (const FilesystemEntry& fileToWatch)
	: watchedPath (fileToWatch)
{
	if (fileToWatch.exists())
	{
		pimpl.reset (new Impl { *this, watchedPath });
	}
	else
	{
		std::stringstream stream;

		stream << "FileWatcher: cannot watch non-existent file "
			   << fileToWatch.getAbsolutePath().string();

		throw std::runtime_error { stream.str() };
	}
}

FileWatcher::FileWatcher() noexcept { }

FileWatcher::~FileWatcher() { }

bool FileWatcher::start()
{
	if (isRunning())
		return true;

	if (! watchedPath.exists())
		return false;

	pimpl.reset (new Impl { *this, watchedPath });

	return isRunning();
}

bool FileWatcher::start (const FilesystemEntry& newPathToWatch)
{
	if (newPathToWatch == watchedPath)
		return start();

	stop();

	if (! newPathToWatch.exists())
		return false;

	watchedPath = newPathToWatch;

	pimpl.reset (new Impl { *this, watchedPath });

	return isRunning();
}

void FileWatcher::stop()
{
	pimpl.reset();
}

bool FileWatcher::isRunning()
{
	return pimpl.get() != nullptr;
}

bool FileWatcher::supportedBySystem() noexcept
{
	return true;
}

FilesystemEntry FileWatcher::getWatchedPath() const noexcept
{
	return watchedPath;
}

}  // namespace files
