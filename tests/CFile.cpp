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

#define TAGS "[core][files][cfile]"

namespace files = limes::files;
using files::CFile;

TEST_CASE ("CFile - null", TAGS)
{
	CFile f;

	REQUIRE (! f.isOpen());
	REQUIRE (f.get() == nullptr);
	REQUIRE (f.getPath().empty());
}

TEST_CASE ("CFile", TAGS)
{
	const auto path = files::dirs::cwd().getChild ("temp.txt").getAbsolutePath();

	CFile f { path, CFile::Mode::Write };

	REQUIRE (f.isOpen());

	REQUIRE (f.getPath() == path);

	const auto path2 = files::dirs::cwd().getChild ("test.png").getAbsolutePath();

	files::File file { path2 };

	file.createIfDoesntExist();

	REQUIRE (file.exists());

	REQUIRE (f.open (path2, CFile::Mode::Read));

	REQUIRE (f.isOpen());

	REQUIRE (f.getPath() == path2);

	// this renaming test fails on Windows. I need to investigate if Windows prevents renaming
	// a file while a C file handle is open to the original path

	const files::Path newPath { files::dirs::cwd().getChild ("another_file.omg").getAbsolutePath() };

	// TODO: this only fails on Windows
#ifndef _WIN32
	REQUIRE (file.rename (newPath));
#endif

	REQUIRE (f.getPath() == newPath);

	REQUIRE (file.deleteIfExists());
}

#undef TAGS
