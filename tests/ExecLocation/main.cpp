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
#include <iostream>
#include <cstdlib>

int main (int, char** argv)
{
	using File = limes::files::File;

	const auto correctPath = File { argv[1] };

	const auto execPath = File::getCurrentExecutable();

	std::cout << "Correct path: " << correctPath.getAbsolutePath() << std::endl;
	std::cout << "Returned path: " << execPath.getAbsolutePath() << std::endl;

	if (correctPath == execPath)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}
