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
#include <vector>
#include <string>
#include <algorithm>
#include <catch2/catch_test_macros.hpp>

#define TAGS "[core][files][directory]"

namespace files = limes::files;

#ifndef __EMSCRIPTEN__
TEST_CASE ("Directory - getChildFile()", TAGS)
{
	const auto cwd = files::dirs::cwd();

	INFO ("CWD: " << cwd.getAbsolutePath());

	REQUIRE (cwd.getChildFile (".") == cwd);
	REQUIRE (cwd.getChildFile ("..") == cwd.getParentDirectory());

	REQUIRE (cwd.getChildFile (".xyz").getFilename() == ".xyz");

	{
		static constexpr auto name = "..xyz";
		REQUIRE (cwd.getChildFile (name).getAbsolutePath() == cwd.getAbsolutePath() / name);
	}

	{
		static constexpr auto name = "...xyz";
		REQUIRE (cwd.getChildFile (name).getAbsolutePath() == cwd.getAbsolutePath() / name);
	}

	REQUIRE (cwd.getChildFile ("./xyz") == cwd.getChildFile ("xyz"));
	REQUIRE (cwd.getChildFile ("././xyz") == cwd.getChildFile ("xyz"));
	REQUIRE (cwd.getChildFile ("../xyz") == cwd.getParentDirectory().getChildFile ("xyz"));
	REQUIRE (cwd.getChildFile (".././xyz") == cwd.getParentDirectory().getChildFile ("xyz"));
	REQUIRE (cwd.getChildFile (".././xyz/./abc") == cwd.getParentDirectory().getChildFile ("xyz/abc"));
	REQUIRE (cwd.getChildFile ("./../xyz") == cwd.getParentDirectory().getChildFile ("xyz"));
	REQUIRE (cwd.getChildFile ("a1/a2/a3/../a4") == cwd.getChildFile ("a1/a2/a4"));
	REQUIRE (cwd.getChildFile ("a1/a2/a3/../../a4") == cwd.getChildFile ("a1/a4"));
	REQUIRE (cwd.getChildFile ("a1/a2/a3/./.././../a4") == cwd.getChildFile ("a1/a4"));
}
#endif

TEST_CASE ("Directory", TAGS)
{
#if ! (defined(_WIN32) || defined(WIN32))
	REQUIRE (files::Directory { "/" }.isDirectory());
#endif

	const auto cwd = files::dirs::cwd();

	REQUIRE (cwd.isCurrentWorkingDirectory());

	const auto dir = cwd.getChildDirectory ("temp");

	dir.deleteIfExists();
	dir.createIfDoesntExist();

	REQUIRE (! dir.isCurrentWorkingDirectory());

	REQUIRE (dir.isEmpty());
	REQUIRE (! dir.containsSubdirectories());

	REQUIRE (dir.getChildDirectories().empty());
	REQUIRE (dir.getChildFiles().empty());
	REQUIRE (dir.getChildSymLinks().empty());

	const std::vector<std::string> dirNames = { "sub1", "sub2", "sub3" };

	for (const auto& name : dirNames)
	{
		const auto subdir = dir.getChildDirectory (name);

		REQUIRE (subdir.createIfDoesntExist());

		REQUIRE (dir.getRelativePath (subdir.getAbsolutePath()) == name);
		REQUIRE (subdir.getName() == name);

		REQUIRE (dir.contains (name));

		const auto link = dir.createChildSymLink (name + "_link",
												  subdir);

		REQUIRE (link.exists());
		REQUIRE (dir.contains (link));
		REQUIRE (link.follow() == subdir);
		REQUIRE (link.references (subdir));
	}

	REQUIRE (dir.containsSubdirectories());
	REQUIRE (! dir.isEmpty());

	for (auto subdir : dir.getChildDirectories())
	{
		REQUIRE (subdir.getDirectory() == subdir);
		REQUIRE (subdir.getParentDirectory() == dir);
		REQUIRE (dir.contains (subdir));

		REQUIRE (std::find (dirNames.begin(),
							dirNames.end(),
							subdir.getName())
				 != dirNames.end());
	}

	{
		size_t num { 0UL };

		for (const auto& entry : dir)
		{
			REQUIRE (dir.contains (entry));
			++num;
		}

		REQUIRE (num == (dirNames.size() * 2UL));  // 1 symlink created to each child dir
	}

	const std::vector<std::string> fileNames = { "file1.txt", "file2.png", "file3.log" };

	for (const auto& name : fileNames)
	{
		const auto file = dir.getChildFile (name);

		REQUIRE (file.createIfDoesntExist());

		REQUIRE (dir.getRelativePath (file.getAbsolutePath()) == name);
		REQUIRE (file.getName() == name);

		REQUIRE (dir.contains (name));

		const auto link = dir.createChildSymLink (name + "_link",
												  file);

		REQUIRE (link.exists());
		REQUIRE (dir.contains (link));
		REQUIRE (link.follow() == file);
		REQUIRE (link.references (file));
	}

	static constexpr auto fileSize = 1024 * 1024;

	for (auto file : dir.getChildFiles())
	{
		REQUIRE (file.getDirectory() == dir);
		REQUIRE (file.getParentDirectory() == dir.getParentDirectory());
		REQUIRE (dir.contains (file));

		REQUIRE (std::find (fileNames.begin(),
							fileNames.end(),
							file.getName())
				 != fileNames.end());

		file.resize (fileSize);
	}

	REQUIRE (dir.sizeInBytes() >= (fileSize * fileNames.size()));

	for (auto link : dir.getChildSymLinks())
	{
		const auto target = link.follow();
		REQUIRE (dir.contains (target));
		REQUIRE (target.isBelow (dir));
	}

	REQUIRE (! dir.contains ("cuwnncncffeohglgreg"));

	REQUIRE (dir.deleteIfExists());

	REQUIRE (dir.isEmpty());
	REQUIRE (! dir.containsSubdirectories());
}

#undef TAGS
