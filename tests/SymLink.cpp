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

TEST_CASE ("SymLink", "[core][files][SymLink]")
{
	using SymLink  = limes::files::SymLink;
	namespace dirs = limes::files::dirs;

	const auto cwd = dirs::cwd();

	const auto target = cwd.getChildFile ("limes_symlink_target.txt");

	target.createIfDoesntExist();

	const auto symLink = SymLink::create (cwd.getChildFile ("link.txt"), target);

	REQUIRE (symLink.has_value());

	REQUIRE (symLink->isSymLink());

	REQUIRE (symLink->references (target));
	REQUIRE (! symLink->references (cwd.getChildFile ("target2.txt")));

	REQUIRE (symLink->follow() == target);

	REQUIRE (! symLink->isDangling());

	const auto symLink2 = SymLink::create (cwd.getChildFile ("link2.txt"), target);

	REQUIRE (symLink2.has_value());

	REQUIRE (symLink->referencesSameLocationAs (*symLink2));

	const auto symLink3 = SymLink::create (cwd.getChildFile ("link3.txt"), symLink2->getAbsolutePath());

	REQUIRE (symLink3.has_value());

	REQUIRE (symLink3->references (symLink2->follow()));
	REQUIRE (symLink3->references (*symLink2));

	REQUIRE (symLink3->follow() == symLink2->follow());
	REQUIRE (symLink3->follow (1) == *symLink2);
	REQUIRE (symLink3->follow (2) == symLink2->follow());

	const auto documents = dirs::userDocuments();

	const auto symLink4 = SymLink::create (documents, target);

	// REQUIRE (symLink4.has_value());

	// REQUIRE (symLink4->isBelow (documents));
	// REQUIRE (symLink4->getName() == target.getName());
	// REQUIRE (symLink4->follow() == target);

	REQUIRE (target.deleteIfExists());

	REQUIRE (symLink->isDangling());

	symLink->deleteIfExists();
	symLink2->deleteIfExists();
	symLink3->deleteIfExists();
	// symLink4->deleteIfExists();
}
