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

#include "./modes.h"
#include <cstdlib>
#include <iostream>

// TODO: add --version

void printUsage()
{
	std::cout << "lfile <mode> [<args...>]\n"
			  << "You can run lfile <mode> help for detailed help for a specific subcommand.\n"
			  << "Available modes:\n";

	for (const auto& mode : limes::files::cli::getAllModes())
		std::cout << mode->getName() << '\n';

	std::cout << std::endl;
}

int main (int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return EXIT_FAILURE;
	}

	const std::string_view modeStr { argv[2] };

	if (modeStr == "help" || modeStr == "Help")
	{
		printUsage();
		return EXIT_SUCCESS;
	}

	auto mode = limes::files::cli::getMode (modeStr);

	if (mode.get() == nullptr)
	{
		std::cout << "Unknown mode requested: '" << mode << "'";
		printUsage();
		return EXIT_FAILURE;
	}

	if (argc > 2)
	{
		const std::string_view subcmd { argv[3] };

		if (subcmd == "help" || subcmd == "Help")
		{
			std::cout << mode->getHelpString() << std::endl;
			return EXIT_SUCCESS;
		}
	}

	if (mode->execute (argc, argv))
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}
