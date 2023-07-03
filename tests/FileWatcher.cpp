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

#include <lfilesystem/lfilesystem.h>
#include <thread>
#include <chrono>
#include <catch2/catch_test_macros.hpp>

#ifdef __APPLE__
#	include <TargetConditionals.h>
#endif

TEST_CASE ("FileWatcher", "[core][files][watcher]")
{
	namespace lf = limes::files;

	{
		lf::FileWatcher w;	// NOLINT

		REQUIRE (! w.isRunning());
		REQUIRE (! w.getWatchedPath().isValid());
	}

	const auto cwd = lf::dirs::cwd();

	const auto file = cwd.getChildFile ("watcher_test.txt");

#if (defined(__ANDROID__) || defined(__EMSCRIPTEN__) || (defined(__APPLE__) && TARGET_OS_IPHONE))

	REQUIRE (! lf::FileWatcher::supportedBySystem());

	file.createIfDoesntExist();

	lf::FileWatcher watcher { file };

	REQUIRE (! watcher.getWatchedPath().isValid());

	REQUIRE (! watcher.isRunning());

	file.deleteIfExists();

#else

	REQUIRE (lf::FileWatcher::supportedBySystem());

	file.deleteIfExists();

	REQUIRE_THROWS (lf::FileWatcher { file });

	REQUIRE (file.createIfDoesntExist());

	lf::FileWatcher watcher { file };

	REQUIRE (watcher.getWatchedPath() == file);

	REQUIRE (watcher.isRunning());

	watcher.stop();

	REQUIRE (! watcher.isRunning());

	watcher.start();

	REQUIRE (watcher.isRunning());

	REQUIRE (watcher.getWatchedPath() == file);

	REQUIRE (file.deleteIfExists());

	// REQUIRE(! watcher.isRunning());

#endif
}
