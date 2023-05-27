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
#include <sstream>
#include <algorithm>
#include <catch2/catch_test_macros.hpp>

#define TAGS "[core][files][special_dirs]"

namespace dirs = limes::files::dirs;

TEST_CASE ("SpecialDirs - temp", TAGS)
{
	const auto temp = dirs::temp();

	REQUIRE (temp.isAbsolutePath());
	REQUIRE (temp.exists());
	REQUIRE (temp.isDirectory());
}

TEST_CASE ("SpecialDirs - PATH", TAGS)
{
	const auto path = dirs::PATH();

	REQUIRE (! path.empty());

	for (const auto& dir : path)
		REQUIRE (dir.isAbsolutePath());
}

TEST_CASE ("SpecialDirs - appendToPATH()", TAGS)
{
	const auto origPath = dirs::PATH();

	REQUIRE (! origPath.empty());

	SECTION ("Append an invalid directory")
	{
		REQUIRE (! dirs::appendToPATH (origPath.front()));
		REQUIRE (dirs::PATH().size() == origPath.size());
	}

	SECTION ("Append a valid directory")
	{
		const auto newDir = dirs::cwd().getChildDirectory ("some_random_path");

		INFO ("Appending directory: " << newDir.getAbsolutePath());

		REQUIRE (dirs::appendToPATH (newDir));

		const auto newPATH = dirs::PATH();

		REQUIRE (std::find (newPATH.begin(), newPATH.end(), newDir) != newPATH.end());

		REQUIRE (newPATH.size() == origPath.size() + 1);
	}
}

TEST_CASE ("SpecialDirs - home", TAGS)
{
	const auto home = dirs::home();

	REQUIRE (home.isAbsolutePath());
	REQUIRE (home.exists());
	REQUIRE (home.isDirectory());
	REQUIRE (! home.isHidden());
}

TEST_CASE ("SpecialDirs - desktop", TAGS)
{
	const auto desktop = dirs::desktop();

	REQUIRE (desktop.isAbsolutePath());
	REQUIRE (desktop.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (desktop.exists());
#endif
}

TEST_CASE ("SpecialDirs - user documents", TAGS)
{
	const auto userDocuments = dirs::userDocuments();

	REQUIRE (userDocuments.isAbsolutePath());
	REQUIRE (userDocuments.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (userDocuments.exists());
#endif
}

TEST_CASE ("SpecialDirs - common documents", TAGS)
{
	const auto commonDocuments = dirs::commonDocuments();

	REQUIRE (commonDocuments.isAbsolutePath());
	REQUIRE (commonDocuments.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (commonDocuments.exists());
#endif
}

TEST_CASE ("SpecialDirs - user app data", TAGS)
{
	const auto userAppData = dirs::userAppData();

	REQUIRE (userAppData.isAbsolutePath());
	REQUIRE (userAppData.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (userAppData.exists());
#endif
}

TEST_CASE ("SpecialDirs - common app data", TAGS)
{
	const auto commonAppData = dirs::commonAppData();

	REQUIRE (commonAppData.isAbsolutePath());
	REQUIRE (commonAppData.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (commonAppData.exists());
#endif
}

TEST_CASE ("SpecialDirs - apps", TAGS)
{
	const auto apps = dirs::apps();

	REQUIRE (apps.isAbsolutePath());
	REQUIRE (apps.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (apps.exists());
#endif
}

TEST_CASE ("SpecialDirs - downloads", TAGS)
{
	const auto downloads = dirs::downloads();

	REQUIRE (downloads.isAbsolutePath());
	REQUIRE (downloads.isDirectory());

#ifndef __EMSCRIPTEN__
	REQUIRE (downloads.exists());
#endif
}

TEST_CASE ("SpecialDirs - working directory", TAGS)
{
	const auto cwd = dirs::cwd();

	REQUIRE (cwd.isAbsolutePath());
	REQUIRE (cwd.exists());
	REQUIRE (cwd.isDirectory());
	REQUIRE (! cwd.isFile());
	REQUIRE (! cwd.isSymLink());

	const auto cwdAtStartup = dirs::cwdAtStartup();

	REQUIRE (cwdAtStartup.isAbsolutePath());
	REQUIRE (cwdAtStartup.exists());
	REQUIRE (cwdAtStartup.isDirectory());

	// this may fail if other tests change the working directory before this one is executed
	CHECK (cwd == cwdAtStartup);

	// should return false if trying to change cwd to what it already was
	REQUIRE (! dirs::setCWD (cwd));

	const auto newCWD = cwd.getChildDirectory ("temp");

	REQUIRE (newCWD.isDirectory());

	REQUIRE (newCWD != cwd);

	newCWD.createIfDoesntExist();

	REQUIRE (newCWD.exists());

	REQUIRE (dirs::setCWD (newCWD));

	REQUIRE (dirs::cwd() == newCWD);

	REQUIRE (dirs::cwdAtStartup() != newCWD);

	REQUIRE (dirs::setCWD (cwd));

	REQUIRE (newCWD.deleteIfExists());

	REQUIRE (! newCWD.exists());
	REQUIRE (cwd.exists());

	// should return false if passed dir doesn't exist
	REQUIRE (! dirs::setCWD (newCWD));
}

#undef TAGS