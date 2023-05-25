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

#define TAGS "[core][files][FilesystemEntry]"

namespace files = limes::files;
namespace dirs	= files::dirs;
using Entry		= files::FilesystemEntry;

TEST_CASE ("FilesystemEntry - relative path", TAGS)
{
	Entry rel { "a/relative/path" };

	REQUIRE (rel.isRelativePath());
	REQUIRE (! rel.isAbsolutePath());
	REQUIRE (rel.isValid());

	REQUIRE (rel.getName() == "path");

	// we want to test that the trailing / is stripped properly
	Entry rel2 { "a/relative/path/" };

	REQUIRE (rel2.isRelativePath());
	REQUIRE (! rel2.isAbsolutePath());
	REQUIRE (rel2.isValid());
	REQUIRE (rel.getPath() == rel2.getPath());
	REQUIRE (rel == rel2);
	REQUIRE (rel2.getName() == "path");

	REQUIRE (rel.getDirectory() == Entry { "a/relative" });
	REQUIRE (rel.getParentDirectory() == Entry { "a" });

	rel.makeAbsoluteRelativeToCWD();
	REQUIRE (rel.isAbsolutePath());
	REQUIRE (rel.isBelow (dirs::cwd()));

	rel2.makeAbsoluteRelativeToCWD();
	REQUIRE (rel2.isAbsolutePath());
	REQUIRE (dirs::cwd().contains (rel2));

	REQUIRE (rel.getAbsolutePath() == rel2.getAbsolutePath());
	REQUIRE (rel == rel2);
}

TEST_CASE ("FilesystemEntry - relative filename", TAGS)
{
	Entry file { "relativeFilename.txt" };

	REQUIRE (file.isRelativePath());
	REQUIRE (file.getName() == file.getPath());

	REQUIRE (file.isFile());
	REQUIRE (file.getFileObject().has_value());

	static constexpr auto newName = "newFilename.png";

	file.changeName (newName);

	REQUIRE (file.getPath() == newName);
	REQUIRE (file.getName() == file.getPath());
}

TEST_CASE ("FilesystemEntry - renaming", TAGS)
{
	static constexpr auto origName = "a_file.txt";

	Entry file { origName };

	REQUIRE (file.getName() == origName);

	file.makeAbsoluteRelativeToCWD();

	REQUIRE (file.getDirectory() == dirs::cwd());

	file.deleteIfExists();
	REQUIRE (file.createIfDoesntExist());
	REQUIRE (file.exists());

	REQUIRE (dirs::cwd().contains (origName));

	static constexpr auto newName = "some_other_file.png";

	REQUIRE (file.rename (newName));

	REQUIRE (file.getName() == newName);
	REQUIRE (file.getDirectory() == dirs::cwd());

	REQUIRE (! dirs::cwd().getChildFile (origName).exists());

	REQUIRE ((file.moveToTrash() || file.deleteIfExists()));
}

TEST_CASE ("FilesystemEntry - absolute paths", TAGS)
{
#if defined(_WIN32) || defined(WIN32)
	const Entry abs { "C:\\an\\absolute\\path\\to\\a\\file.txt" };
#else
	const Entry abs { "/an/absolute/path/to/a/file.txt" };
#endif

	REQUIRE (abs.isAbsolutePath());
	REQUIRE (! abs.isRelativePath());

	REQUIRE (abs.getName() == "file.txt");

#if defined(_WIN32) || defined(WIN32)
	REQUIRE (abs.getDirectory().getAbsolutePath() == "C:\\an\\absolute\\path\\to\\a");
	REQUIRE (abs.getParentDirectory().getAbsolutePath() == "C:\\an\\absolute\\path\\to");
#else
	REQUIRE (abs.getDirectory().getAbsolutePath() == "/an/absolute/path/to/a");
	REQUIRE (abs.getParentDirectory().getAbsolutePath() == "/an/absolute/path/to");
#endif

	REQUIRE (abs.isFile());
	REQUIRE (abs.getFileObject().has_value());
}

TEST_CASE ("FilesystemEntry - dotfile paths", TAGS)
{
	SECTION ("Relative")
	{
		const Entry e { ".zshrc" };	 // NOLINT

		REQUIRE (e.isHidden());
	}

#if ! (defined(_WIN32) || defined(WIN32))
	SECTION ("Absolute")
	{
		const auto e2 = dirs::cwd().getChild (".vimrc");  // NOLINT

		e2.createIfDoesntExist();

		REQUIRE (e2.isHidden());

		e2.deleteIfExists();
	}
#endif
}

TEST_CASE ("FilesystemEntry - .", TAGS)
{
	SECTION (". only")
	{
		const Entry e { "." };

		REQUIRE (e.isRelativePath());
		REQUIRE (e == dirs::cwd());
		REQUIRE (e.getName() == ".");
	}

	SECTION (". in a nested directory chain")
	{
		SECTION ("Absolute")
		{
			const Entry e1 { "/some/absolute/./path/" };
			const Entry e2 { "/some/absolute/path/" };

			INFO ("e1: " << e1.getPath());
			INFO ("e2: " << e2.getPath());

			REQUIRE (e1.isAbsolutePath());
			REQUIRE (e2.isAbsolutePath());
			REQUIRE (e1.getName() == e2.getName());
			REQUIRE (e1.getDirectory() == e2.getDirectory());
			REQUIRE (e1.getPath() == e2.getPath());
			REQUIRE (e1 == e2);
		}

		SECTION ("Relative")
		{
			const Entry e3 { "a/rel/path" };
			const Entry e4 { "a/./rel/./path" };

			REQUIRE (e3.isRelativePath());
			REQUIRE (e4.isRelativePath());
			REQUIRE (e3.getName() == e4.getName());
			REQUIRE (e3 == e4);
			REQUIRE (e3.getPath() == e4.getPath());
			REQUIRE (e3.isRelativePath());
			REQUIRE (e4.isRelativePath());
		}
	}

	SECTION (". at the start of a relative path")
	{
		Entry e5 { "./my_executable" };

		REQUIRE (e5.isRelativePath());

		const auto e6 = dirs::cwd().getChild ("my_executable");

		REQUIRE (e5.getName() == e6.getName());

		e5.makeAbsoluteRelativeToCWD();

		REQUIRE (e5.getPath() == e6.getPath());
	}
}

TEST_CASE ("FilesystemEntry - ..", TAGS)
{
	SECTION (".. only")
	{
		const Entry e { ".." };

		REQUIRE (e.isRelativePath());
		REQUIRE (e == dirs::cwd().getParentDirectory());
		REQUIRE (e.getName() == "..");
	}

	SECTION (".. in a nested directory chain")
	{
		SECTION ("Absolute")
		{
			const Entry e1 { "/some/absolute/../path/" };
			const Entry e2 { "/some/path" };

			REQUIRE (e1.getName() == e2.getName());
			REQUIRE (e1.getDirectory() == e2.getDirectory());
			REQUIRE (e1.getParentDirectory() == e2.getParentDirectory());
			REQUIRE (e1.getPath() == e2.getPath());
			REQUIRE (e1 == e2);
		}

		SECTION ("Relative")
		{
			const Entry e3 { "a/path/" };
			const Entry e4 { "a/rel/../path" };

			REQUIRE (e3.getName() == e4.getName());
			REQUIRE (e3 == e4);
			REQUIRE (e3.getPath() == e4.getPath());
			REQUIRE (e3.isRelativePath());
			REQUIRE (e4.isRelativePath());
		}
	}
}

#if ! (defined(_WIN32) || defined(WIN32))

TEST_CASE ("FilesystemEntry - tilde in path", TAGS)
{
	SECTION ("Tilde only")
	{
		const Entry e { "~" };

		REQUIRE (e.exists());
		REQUIRE (e.isAbsolutePath());
		REQUIRE (e.isDirectory());
	}

	SECTION ("Tilde with following subdir")
	{
		const Entry e2 { "~/abc" };

		REQUIRE (e2.isAbsolutePath());
		REQUIRE (e2 == dirs::home().getChild ("abc"));
	}

	SECTION ("Tilde with leading username")
	{
		const Entry e3 { "~username/abc" };

		REQUIRE (e3.isAbsolutePath());
	}
}

#endif /* ! Windows */

TEST_CASE ("FilesystemEntry - /", TAGS)
{
	const Entry e { "/" };

	REQUIRE (e.isAbsolutePath());
	REQUIRE (e.isDirectory());
	REQUIRE (e.exists());
}

#if (defined(_WIN32) || defined(WIN32))

TEST_CASE ("FilesystemEntry - backslash", TAGS)
{
	const Entry e { "\\" };

	REQUIRE (e.isAbsolutePath());
	REQUIRE (e.isDirectory());
	REQUIRE (e.exists());
}

#endif /* Windows */

TEST_CASE ("FilesystemEntry - copying", TAGS)
{
	const auto cwd = dirs::cwd();

	const auto file = cwd.getChild ("file.temp.txt");

	const auto origPath = file.getAbsolutePath();

	REQUIRE (file.getDirectory() == cwd);

	file.createIfDoesntExist();

	// cannot copyToDirectory into same directory
	REQUIRE (! file.copyToDirectory (file.getDirectory())
				   .has_value());

	const auto destDir = file.getDirectory().getChildDirectory ("subdirectory");

	destDir.deleteIfExists();

	const auto copy = file.copyToDirectory (destDir);

	REQUIRE (copy.has_value());
	REQUIRE (copy->exists());
	REQUIRE (copy->getName() == file.getName());  // NOLINT
	REQUIRE (copy->getDirectory() == destDir);
	REQUIRE (destDir.exists());

	REQUIRE (destDir.deleteIfExists());

	const auto copy2 = cwd.getChild ("file.copy.txt");

	copy2.deleteIfExists();

	REQUIRE (! copy2.exists());

	REQUIRE (file.copyTo (copy2));

	REQUIRE (copy2.exists());

	const auto copy3 = cwd.getChild ("file.final.txt");

	copy3.deleteIfExists();

	REQUIRE (! copy3.exists());

	copy3.copyFrom (file);

	REQUIRE (copy3.exists());

	REQUIRE (file.exists());
	REQUIRE (file.getAbsolutePath() == origPath);

	file.deleteIfExists();
	copy2.deleteIfExists();
	copy3.deleteIfExists();
}

TEST_CASE ("FilesystemEntry - null", TAGS)
{
	Entry empty;

	REQUIRE (! empty.isValid());
	REQUIRE (! empty.exists());
	REQUIRE (! empty.createIfDoesntExist());
	REQUIRE (! empty.deleteIfExists());
	REQUIRE (! empty.moveToTrash());
	REQUIRE (! empty.revealToUserInFileBrowser());

	REQUIRE (empty.getPath().empty());
	REQUIRE (empty.getAbsolutePath().empty());
	REQUIRE (empty.getName().empty());

	REQUIRE (empty.getDirectory().getAbsolutePath().empty());

	REQUIRE (empty.getParentDirectory().getAbsolutePath().empty());

	REQUIRE (! empty.isAbsolutePath());
	REQUIRE (! empty.isRelativePath());

	REQUIRE (empty.sizeInBytes() == 0);

	REQUIRE (! empty.getVolume().has_value());
	REQUIRE (! empty.getFileObject().has_value());
	REQUIRE (! empty.getDirectoryObject().has_value());
	REQUIRE (! empty.getSymLinkObject().has_value());
}

TEST_CASE ("FilesystemEntry - invalid paths", TAGS)
{
	SECTION (":: only")
	{
		const Entry e { "::" };

		REQUIRE (! e.isValid());
	}

	SECTION (":: as part of larger path")
	{
		const Entry e { "some/relative/path::name" };

		REQUIRE (! e.isValid());

		const Entry e2 { "/an/absolute::/path/" };

		REQUIRE (! e2.isValid());
	}
}

TEST_CASE ("FilesystemEntry - case sensitivity", TAGS)
{
	const Entry e1 { "filename.txt" };
	const Entry e2 { "FILENAME.txt" };

	const Entry e3 { "some/relative/path.txt" };
	const Entry e4 { "sOme/ReLative/PATh.txt" };

	if (files::volume::caseSensitive())
	{
		REQUIRE (e1 != e2);
		REQUIRE (e3 != e4);
	}
	else
	{
		REQUIRE (e1 == e2);
		REQUIRE (e3 == e4);
	}
}

TEST_CASE ("FilesystemEntry - siblings", TAGS)
{
	const auto cwd = dirs::cwd();

	SECTION ("Filenames")
	{
		const auto entry1 = cwd.getChild ("some_file.txt");

		const auto entry2 = entry1.getSibling ("another_file.jpeg");

		REQUIRE (entry2.getDirectory() == cwd);
	}

	SECTION ("Sibling directories")
	{
		const auto entry1 = cwd.getChildDirectory ("foo");

		const auto entry2 = entry1.getSibling ("bar");

		REQUIRE (entry2.getDirectory() == entry1.getDirectory());
	}
}

#undef TAGS
