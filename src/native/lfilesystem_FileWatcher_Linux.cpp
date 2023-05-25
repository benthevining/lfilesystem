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

#if ! LIMES_LINUX
#	error
#endif

#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <memory>
#include <thread>
#include "lfilesystem_FileWatcher.h"
#include "lfilesystem_FilesystemEntry.h"

namespace limes::files
{

class LFILE_NO_EXPORT FileWatcher::Impl final
{
public:
	Impl (FileWatcher& parent, const FilesystemEntry& fileToWatch)
		: watcher (parent), watchedPath (fileToWatch.getAbsolutePath()), file_descriptor (inotify_init())
	{
		if (file_descriptor < 0)
			throw std::runtime_error { "FileWatcher failed to initialize" };

		watch_descriptor = inotify_add_watch (file_descriptor,
											  fileToWatch.getAbsolutePath().c_str(),
											  IN_ACCESS | IN_ATTRIB | IN_CLOSE | IN_CREATE | IN_DELETE | IN_DELETE_SELF
												  | IN_MODIFY | IN_MOVE | IN_MOVE_SELF | IN_OPEN);

		if (watch_descriptor < 0)
			throw std::runtime_error { "FileWatcher failed to initialize" };

		updaterThread.add (*this);

		// FD_ZERO (&descriptorSet);

		// timeOut.tv_sec	= 0;
		// timeOut.tv_usec = 0;
	}

	~Impl()
	{
		if (file_descriptor >= 0)
		{
			if (watch_descriptor >= 0)
			{
				inotify_rm_watch (file_descriptor, watch_descriptor);
				updaterThread.remove (*this);
			}

			close (file_descriptor);
		}
	}

private:
	void update()
	{
		// FD_SET (file_descriptor, &descriptorSet);

		// const auto ret = select (file_descriptor + 1, &descriptorSet, nullptr, nullptr, &timeOut);

		// if (ret < 0)
		// 	return;

		// if (! FD_ISSET (file_descriptor, &descriptorSet))
		// 	return;

		std::memset (&dataBuffer[0], 0, BUF_SIZE);

		const auto len = read (file_descriptor, dataBuffer, BUF_SIZE);

		for (ssize_t i = 0; i < len;)
		{
			const auto* prevent = reinterpret_cast<const struct inotify_event*> (&dataBuffer[i]);

			if (prevent == nullptr)
				return;

			Path path;

			if (const auto* name = prevent->name)
				path = Path { name };

			handleEvent (static_cast<std::uint8_t> (prevent->mask), path);

			i += sizeof (struct inotify_event) + prevent->len;
		}
	}

	void handleEvent (std::uint8_t action, const Path& path) const
	{
		FilesystemEntry file { path };

		file.makeAbsoluteRelativeTo (watchedPath);

		if (IN_ACCESS & action)
		{
			watcher.fileAccessed (file);
			return;
		}

		if (IN_ATTRIB & action)
		{
			watcher.fileMetadataChanged (file);
			return;
		}

		if (IN_CLOSE & action)
		{
			watcher.fileHandleClosed (file);
			return;
		}

		if (IN_CREATE & action)
		{
			watcher.fileCreated (file);
			return;
		}

		if (IN_MODIFY & action)
		{
			watcher.fileModified (file);
			return;
		}

		if (IN_OPEN & action)
		{
			watcher.fileOpened (file);
			return;
		}

		if (IN_MOVE_SELF & action || IN_MOVE & action)
		{
			watcher.fileMoved (file);
			return;
		}

		if (IN_DELETE & action || IN_DELETE_SELF & action)
			watcher.fileDeleted (file);
	}

	FileWatcher& watcher;

	Path watchedPath;

	int file_descriptor, watch_descriptor;

	static constexpr auto BUF_SIZE = (sizeof (struct inotify_event) + FILENAME_MAX) * 1024;

	char dataBuffer[BUF_SIZE] = { 0 };

	// fd_set descriptorSet;

	// struct timeval timeOut;

	/*---------------------------------------------------------------------------------------------------------------------*/

	struct UpdaterThread final
	{
	public:
		void add (Impl& watcher)
		{
			{
				const std::lock_guard g { mutex };

				watchers.emplace_back (&watcher);
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

							if (! thread->get_stop_token().stop_requested())
								std::this_thread::sleep_for (std::chrono::milliseconds (250));
						}
					} });

				thread->detach();
			}
		}

		void remove (Impl& watcher)
		{
			const std::lock_guard g { mutex };

			watchers.erase (std::find (std::begin (watchers), std::end (watchers), &watcher));

			if (watchers.empty())
				thread.reset();
		}

	private:
		std::vector<Impl*> watchers;

		std::mutex mutex;

		std::unique_ptr<std::jthread> thread;
	};

	static UpdaterThread updaterThread;
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
