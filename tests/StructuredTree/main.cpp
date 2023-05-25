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
#include <catch2/catch_test_macros.hpp>

/** This executable observes the structure of a predefined
	directory tree. This executable must be run via CTest;
	the CreateTree.cmake script should be run before executing
	this test in order to prepare the filesystem for this
	test to inspect.

	Running this executable on its own outside CTest will
	probably (expectedly) result in test failures.
 */

TEST_CASE ("Prebuilt directory tree", "[core][files]")
{
	const auto treeRoot = limes::files::dirs::cwd();

	REQUIRE (treeRoot.exists());
	REQUIRE (treeRoot.isDirectory());

	REQUIRE (treeRoot.getName() == "FileTree");

	REQUIRE (treeRoot.containsSubdirectories());

	REQUIRE (treeRoot.getChildFiles (false).size() == 3);

	REQUIRE (treeRoot.contains ("example.txt"));
	REQUIRE (treeRoot.getChildFile ("sample.omg").exists());
	REQUIRE (treeRoot.contains (".trial"));

	const auto subdirs = treeRoot.getChildDirectories (false);

	REQUIRE (subdirs.size() == 3);

	for (const auto& subdir : subdirs)
	{
		REQUIRE (subdir.exists());
		REQUIRE (subdir.isDirectory());
		REQUIRE (treeRoot.contains (subdir));

		const auto dirName = subdir.getName();

		if (dirName == "Baz")
		{
			REQUIRE (subdir.isEmpty());
			continue;
		}

		if (dirName == "Bar")
		{
			REQUIRE (subdir.contains ("foo"));
			REQUIRE (subdir.contains ("Bar.cmake"));
			continue;
		}

		REQUIRE (dirName == "Foo");

		REQUIRE (! subdir.containsSubdirectories());

		REQUIRE (subdir.getAllChildren().size() == 2);

		REQUIRE (subdir.contains ("hello.txt"));
		REQUIRE (subdir.contains ("world.png"));

		const auto file = subdir.getChildFile ("hello.txt");

		REQUIRE (file.loadAsString() == "The quick brown fox jumps over the lazy dog.");
	}
}
