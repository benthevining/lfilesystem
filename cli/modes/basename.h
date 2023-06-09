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

#pragma once

#include "../modes.h"
#include <lfilesystem/lfilesystem.h>
#include <string_view>
#include <iostream>

namespace limes::files::cli::modes
{

/** TODO: suffix business...
	Should the path be printed unquoted?
 */
class Basename final : public Mode
{
public:
	[[nodiscard]] std::string_view getName() const final
	{
		return "basename";
	}

	void outputHelp() const final
	{
		std::cout << "Usage:\n\n"
				  << "basename <path>\n\n"
				  << "Prints the directory portion of the given path."
				  << std::endl;
	}

	[[nodiscard]] bool execute (int argc, char** argv) const final
	{
		if (argc < 2)
		{
			outputHelp();
			return false;
		}

		const FilesystemEntry path { argv[1] };

		const auto parentPath = path.isDirectory() ? path.getParentDirectory() : path.getDirectory();

		std::cout << parentPath.getPath() << std::endl;

		return true;
	}
};

}
