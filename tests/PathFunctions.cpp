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

#define TAGS "[core][files][paths]"

namespace files = limes::files;
using Path		= files::Path;

TEST_CASE ("isValidPath()", TAGS)
{
	REQUIRE (! files::isValidPath (Path {}));
	REQUIRE (! files::isValidPath ("::"));
	REQUIRE (! files::isValidPath ("some/p::ath"));

	REQUIRE (files::isValidPath ("."));
	REQUIRE (files::isValidPath (".."));

	REQUIRE (files::isValidPath (".abc"));
	REQUIRE (files::isValidPath ("..abc"));

	REQUIRE (files::isValidPath ("./"));
	REQUIRE (files::isValidPath ("../"));
}

TEST_CASE ("normalizePath()", TAGS)
{
	REQUIRE (files::normalizePath (Path {}).empty());
	REQUIRE (files::normalizePath ("::").empty());

	REQUIRE (files::normalizePath (".") == ".");
	REQUIRE (files::normalizePath ("./") == ".");

	REQUIRE (files::normalizePath ("..") == "..");

	REQUIRE (files::normalizePath ("/some/./path") == "/some/path");

	REQUIRE (files::normalizePath ("some/other/../path") == "some/path");

	REQUIRE (files::normalizePath ("a/path/with/../.././some/complexity/./") == "a/some/complexity");

	REQUIRE (files::normalizePath ("walking/back/wards/../..") == "walking");

	REQUIRE (files::normalizePath ("a/./rather/complex/.././path/to/normalize/../..") == "a/rather/path");
}

TEST_CASE ("largestCommonPrefix()", TAGS)
{
	REQUIRE (files::largestCommonPrefix ("path/1", "path/2") == "path");
	REQUIRE (files::largestCommonPrefix ("/path/1", "/path/2") == "/path");

	REQUIRE (files::largestCommonPrefix ("foo/bar/baz", "for/dar/dot").empty());

	REQUIRE (files::largestCommonPrefix ("some/example/path", "some/example/path") == "some/example/path");

	REQUIRE (files::largestCommonPrefix ("/", "/") == "/");

	REQUIRE (files::largestCommonPrefix ("/foo", "/bar") == "/");

	REQUIRE (files::largestCommonPrefix ("a/longer/more/complex/path/example", "a/longer/much/more/complex/path/example") == "a/longer");
}

#undef TAGS
