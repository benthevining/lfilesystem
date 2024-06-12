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
#include <string_view>

// TODO: add --version

static void printUsage()
{
	std::cout << "Usage:\n\n"
			  << "lfile <mode> [<args...>]\n\n"
			  << "You can run lfile <mode> help for detailed help for a specific subcommand.\n"
			  << "Available modes:\n\n";

	for (const auto& mode : limes::files::cli::getAllModes())
		std::cout << "  * " << mode->getName() << '\n';

	std::cout << std::endl;
}

static inline bool isHelpSubcommand (std::string_view subcmd) noexcept
{
	return subcmd == "help" || subcmd == "Help" || subcmd == "--help" || subcmd == "-h" || subcmd == "-help";
}

int main (int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return EXIT_FAILURE;
	}

	const std::string_view modeStr { argv[1] };

	if (isHelpSubcommand (modeStr))
	{
		printUsage();
		return EXIT_SUCCESS;
	}

	const auto* mode = limes::files::cli::getMode (modeStr);

	if (mode == nullptr)
	{
		std::cout << "Unknown mode requested: '" << modeStr << "'\n\n";
		printUsage();
		return EXIT_FAILURE;
	}

	if (argc > 2)
	{
		const std::string_view subcmd { argv[2] };

		if (isHelpSubcommand (subcmd))
		{
			mode->outputHelp();
			return EXIT_SUCCESS;
		}
	}

	try
	{
		// consume the 'lfile' from the command line argument list
		if (mode->execute (argc - 1, argv + 1))
			return EXIT_SUCCESS;

		return EXIT_FAILURE;
	}
	catch(...)
	{
		return EXIT_FAILURE;
	}
}
