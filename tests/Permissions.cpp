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

TEST_CASE ("Permissions", "[core][files][misc]")
{
	using limes::files::Permissions;

	const Permissions empty;

	REQUIRE (empty.isUnknownOrEmpty());

	const auto withRead = empty.withRead (Permissions::Scope::Owner);

	REQUIRE (! withRead.isUnknownOrEmpty());
	REQUIRE (withRead != empty);
	REQUIRE (withRead.hasRead (Permissions::Scope::Owner));
	REQUIRE (! withRead.hasRead (Permissions::Scope::Group));

	REQUIRE (withRead.toString() == "r--------");

	const auto allPerms = Permissions::fromString ("rwxrwxrwx");

	REQUIRE (! allPerms.isUnknownOrEmpty());
	REQUIRE (allPerms.hasAll (Permissions::Scope::All));

	REQUIRE (Permissions::fromString ("wuncenccwg").isUnknownOrEmpty());

	REQUIRE (! Permissions::ownerAll().hasRead (Permissions::Scope::Group));

	REQUIRE (Permissions::groupAll().hasAll (Permissions::Scope::Group));

	const auto file = limes::files::dirs::cwd().getChildFile ("temp.txt");
	file.createIfDoesntExist();

	file.setPermissions (Permissions::all());

	REQUIRE (file.getPermissions().hasAll (Permissions::Scope::All));

	file.setPermissions (Permissions::ownerAll());

	// TODO: reducing permissions on Windows seems to fail
	REQUIRE (! file.getPermissions().hasRead (Permissions::Scope::Group));

	file.setPermissions (Permissions::all());

	REQUIRE (file.getPermissions().hasAll());

	file.deleteIfExists();
}
