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

#include <limes_files.h>
#include <catch2/catch_test_macros.hpp>

#ifndef LIMES_TEST_DYLIB_PATH
#	error LIMES_TEST_DYLIB_PATH must be defined!
#endif

TEST_CASE ("DynamicLibrary", "[core][files][DynamicLibrary]")
{
	limes::files::DynamicLibrary lib { LIMES_TEST_DYLIB_PATH };

	REQUIRE (lib.isOpen());

	REQUIRE (lib.getFile() == limes::files::File { LIMES_TEST_DYLIB_PATH });
}
