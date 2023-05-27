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
#include <algorithm>
#include <catch2/catch_test_macros.hpp>

TEST_CASE ("Volume", "[core][files][volume]")
{
	namespace lf = limes::files;

	const auto cwd = lf::dirs::cwd();

	REQUIRE (cwd.exists());

	const lf::Volume currentVolume { cwd.getAbsolutePath() };

	REQUIRE (currentVolume.contains (cwd));
	REQUIRE (currentVolume.totalBytes() > 1024 * 1024);
	REQUIRE (currentVolume.bytesFree() > 0);

	REQUIRE (! currentVolume.getPath().empty());

	REQUIRE (*cwd.getVolume() == currentVolume);

	const auto file = cwd.getChildFile ("test_file.txt");
	file.createIfDoesntExist();

	REQUIRE (lf::Volume { file } == currentVolume);

	file.deleteIfExists();

	const lf::Volume homeVolume { lf::dirs::home() };

	{
		const lf::Volume copy { homeVolume.getPath() };

		REQUIRE (copy.getPath() == homeVolume.getPath());
		REQUIRE (copy == homeVolume);
	}

	REQUIRE (homeVolume.getType() == lf::Volume::Type::HardDisk);

	REQUIRE (! homeVolume.isReadOnly());

	const auto allVolumes = lf::Volume::getAll();

	REQUIRE (! allVolumes.empty());

	REQUIRE (std::find (allVolumes.begin(),
						allVolumes.end(),
						currentVolume)
			 != allVolumes.end());

	REQUIRE (std::find (allVolumes.begin(),
						allVolumes.end(),
						homeVolume)
			 != allVolumes.end());

	for (const auto& v : allVolumes)
	{
		lf::FilesystemEntry entry { v.getPath() };

		INFO ("Volume path: " << entry.getPath());

		REQUIRE (entry.exists());
		CHECK_NOFAIL (! entry.isFile());
		REQUIRE (entry.isAbsolutePath());

		const auto v2 = entry.getVolume();

		REQUIRE (v2.has_value());
		REQUIRE (*v2 == v);
	}
}
