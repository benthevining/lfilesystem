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
#include <string>
#include <catch2/catch_test_macros.hpp>

#define TAGS "[core][files][file]"

namespace files = limes::files;
using files::File;

TEST_CASE ("File - null", TAGS)
{
	const File f {};

	REQUIRE (! f.isValid());
	REQUIRE (! f.exists());
	REQUIRE (! f.createIfDoesntExist());
	REQUIRE (! f.deleteIfExists());
	REQUIRE (f.isFile());
	REQUIRE (! f.hasFileExtension());
	REQUIRE (f.getFilename().empty());
	REQUIRE (! f.isMacOSBundle());
	REQUIRE (f.loadAsString().empty());
	REQUIRE (f.getHardLinkCount() == 0);
	REQUIRE (! f.duplicate().has_value());
	REQUIRE (f.getPermissions().isUnknownOrEmpty());
	REQUIRE (f.sizeInBytes() == 0);
	REQUIRE (! f.getVolume().has_value());
}

TEST_CASE ("File - executable path", TAGS)
{
	const auto executable = File::getCurrentExecutable();

	REQUIRE (File::getCurrentModule() == executable);

	REQUIRE (executable.isValid());
	REQUIRE (executable.exists());
	REQUIRE (! executable.isHidden());
	REQUIRE (executable.isFile());
	REQUIRE (! executable.isMacOSBundle());
	REQUIRE (executable.getHardLinkCount() == 1);
	REQUIRE (executable.getPermissions().hasExecute (files::Permissions::Scope::Owner));

	const auto v = executable.getVolume();

	REQUIRE (v.has_value());
	REQUIRE (*v == files::Volume {});
}

TEST_CASE ("File - to/from CFile", TAGS)
{
	const auto cwd = limes::files::dirs::cwd();

	const auto file = cwd.getChildFile ("test.txt");

	file.createIfDoesntExist();

	REQUIRE (file.exists());

	const auto cFile = file.getCfile();

	REQUIRE (cFile.isOpen());

	const File file2 { cFile.getPath() };

	REQUIRE (file == file2);
	REQUIRE (file == cFile.getFile());

	file.deleteIfExists();
}

TEST_CASE ("File", TAGS)
{
	const auto cwd = limes::files::dirs::cwd();

	static constexpr auto filename = "file_test.txt";

	const auto file = cwd.getChildFile (filename);

	REQUIRE (file.getFilename (true) == filename);
	REQUIRE (file.getFilename (false) == "file_test");
	REQUIRE (file.getFileExtension() == ".txt");
	REQUIRE (file.hasFileExtension());
	REQUIRE (file.hasFileExtension (".txt"));
	REQUIRE (file.hasFileExtension ("txt"));
	REQUIRE (! file.hasFileExtension (".png"));
	REQUIRE (! file.hasFileExtension ("."));

	static constexpr auto filename2 = "other_file";

	const auto file2 = cwd.getChildFile (filename2);

	REQUIRE (! file2.hasFileExtension());
	REQUIRE (file2.getFileExtension().empty());
	REQUIRE (file2.getFilename (true) == filename2);

	file.deleteIfExists();

	REQUIRE (file.loadAsString().empty());
	REQUIRE (file.loadAsLines().empty());

	file.createIfDoesntExist();

	REQUIRE (! file.isMacOSBundle());

	REQUIRE (file.loadAsString().empty());

	static constexpr auto test_content = "Good morning world, and all who inhabit it!";

	REQUIRE (file.overwrite (test_content));
	REQUIRE (file.loadAsString() == test_content);
	REQUIRE (file.sizeInBytes() == 43);

	REQUIRE (file.copyTo (file2));
	REQUIRE (file2.sizeInBytes() == 43);
	REQUIRE (file2.loadAsString() == test_content);

	static constexpr auto extra_content = "Good night world, and all who inhabit it!";

	REQUIRE (file.append (extra_content));
	REQUIRE (file.sizeInBytes() == 84);

	std::string fileContent { test_content };
	fileContent += extra_content;

	REQUIRE (file.loadAsString() == fileContent);

	static constexpr auto first_content = "I'm ready, I'm ready, I'm ready...";

	REQUIRE (file.prepend (first_content));

	std::string fullContent { first_content };
	fullContent += fileContent;

	REQUIRE (file.loadAsString() == fullContent);

	REQUIRE (file.getHardLinkCount() == 1);

	static constexpr auto link_name = "hard_link_test";

	const auto file3 = file.createHardLink (cwd.getChildFile (link_name));

	REQUIRE (file3.has_value());

	REQUIRE (*file3 == file);
	REQUIRE (file == *file3);

	REQUIRE (file3->getName() == link_name);

	REQUIRE (file.getHardLinkCount() == 2);
	REQUIRE (file3->getHardLinkCount() == 2);

	REQUIRE (file.overwrite (""));
	REQUIRE (file.loadAsString().empty());

	file.deleteIfExists();
	file2.deleteIfExists();
	file3->deleteIfExists();
}

#ifndef LIMES_TEST_DATA_DIR
#	error LIMES_TEST_DATA_DIR not defined!
#endif

TEST_CASE ("File IO", TAGS)
{
	files::Directory dataDir { LIMES_TEST_DATA_DIR };

	REQUIRE (dataDir.isDirectory());
	REQUIRE (dataDir.exists());
	REQUIRE (! dataDir.containsSubdirectories());
	REQUIRE (! dataDir.isEmpty());

	{
		const auto stringFile = dataDir.getChildFile ("string.txt");

		REQUIRE (stringFile.exists());

		const auto content = stringFile.loadAsString();

		REQUIRE (content == R"(This file is one string that will be loaded by a unit test and
 compared for

\equality to $%this document.

^}

I want to use some special \\\ characters \r\n to try to trip up
the code I'm testing.

¯\_(ツ)_/¯
)");
	}

	{
		const auto linesFile = dataDir.getChildFile ("lines.txt");

		REQUIRE (linesFile.exists());

		for (const auto& line : linesFile)
		{
			REQUIRE (line == std::string { "This file will be loaded by a unit test that will iterate over each line, validating that files can be split into lines correctly." });
		}
	}
}

#undef TAGS
