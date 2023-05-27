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

#include <windows.h>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <memory>
#include <thread>
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
		WCHAR path[_MAX_PATH] = { 0 };
		wcsncpy (path, fileToWatch.getAbsolutePath().wstring().data(), _MAX_PATH - 1);

		fileHandle = CreateFileW (path, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
								  nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

		if (fileHandle == INVALID_HANDLE_VALUE)
			throw std::runtime_error { "FileWatcher failed to initialize" };

		updaterThread.add (*this);
	}

	~Impl()
	{
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			CancelIoEx (fileHandle, nullptr);
			updaterThread.remove (*this);
		}

		CloseHandle (fileHandle);
	}

private:
	void update()
	{
		std::memset (buffer, 0, heapSize);

		DWORD bytesOut = 0;

		const auto success = ReadDirectoryChangesW (fileHandle, buffer, heapSize, true,
													FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_LAST_ACCESS | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_SECURITY,
													&bytesOut, nullptr, nullptr);

		if (! (success && bytesOut > 0))
			return;

		for (auto* rawData = buffer; rawData != &buffer[heapSize - 1];)
		{
			const auto* fni = reinterpret_cast<FILE_NOTIFY_INFORMATION*> (rawData);

			if (fni == nullptr)
				return;

			handleEvent (fni->Action, fni->FileName);

			if (fni->NextEntryOffset > 0)
				rawData += fni->NextEntryOffset;
			else
				return;
		}
	}

	void handleEvent (DWORD type, const Path& path) const
	{
		FilesystemEntry file { path };

		file.makeAbsoluteRelativeTo (watchedPath);

		switch (type)
		{
			case (FILE_ACTION_ADDED) :
			{
				watcher.fileCreated (file);
				return;
			}
			case (FILE_ACTION_MODIFIED) :
			{
				watcher.fileModified (file);
				return;
			}
			case (FILE_ACTION_RENAMED_OLD_NAME) :
				[[fallthrough]];
			case (FILE_ACTION_RENAMED_NEW_NAME) :
			{
				watcher.fileMoved (file);
				return;
			}
			case (FILE_ACTION_REMOVED) :
				watcher.fileDeleted (file);
		}
	}

	FileWatcher& watcher;

	Path watchedPath;

	HANDLE fileHandle;

	static constexpr auto heapSize		   = 16 * 1024;
	std::uint8_t		  buffer[heapSize] = {};

	/*---------------------------------------------------------------------------------------------------------------------*/

	struct LFILE_EXPORT UpdaterThread final
	{
	public:
		void add (Impl& watcher_)
		{
			{
				const std::lock_guard g { mutex };

				watchers.emplace_back (&watcher_);
			}

			if (thread.get() == nullptr)
			{
				thread.reset (new std::jthread {
					[this]
					{
						while (! thread->get_stop_token().stop_requested())
						{
							{
								const std::lock_guard g_ { mutex };

								for (auto* w : watchers)
									w->update();
							}

							if (thread->get_stop_token().stop_requested())
							{
								thread.reset();
								return;
							}
							else
								std::this_thread::sleep_for (std::chrono::milliseconds (250));
						}
					} });

				thread->detach();
			}
		}

		void remove (Impl& watcher_)
		{
			const std::lock_guard g { mutex };

			watchers.erase (std::find (std::begin (watchers), std::end (watchers), &watcher_));

			if (watchers.empty())
				thread->request_stop();
		}

	private:
		std::vector<Impl*> watchers;

		std::mutex mutex;

		std::unique_ptr<std::jthread> thread;
	};

	UpdaterThread updaterThread;  // should be static, but that caused unresolved external symbol errors in MSVC...
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
