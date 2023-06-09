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

/** TODO:

	-b Number the non-blank output lines, starting at 1.

	-e Display non-printing characters (see the -v option), and display a dollar sign ('$') at the end of each line.

	-l Set an exclusive advisory lock on the standard output file descriptor.  This lock is set using fcntl(2) with
		the F_SETLKW command.  If the output file is already locked, cat will block until the lock is acquired.

	-n Number the output lines, starting at 1.

	-s Squeeze multiple adjacent empty lines, causing the output to be single spaced.

	-t Display non-printing characters (see the -v option), and display tab characters as '^I'.

	-u Disable output buffering.

	-v Display non-printing characters so they are visible.  Control characters print as '^X' for control-X; the delete
		character (octal 0177) prints as '^?'.  Non-ASCII characters (with the high bit set) are printed as 'M-' (for
		meta) followed by the character for the low 7 bits.
 */
class Cat final : public Mode
{
public:
	[[nodiscard]] std::string_view getName() const final
	{
		return "cat";
	}

	void outputHelp() const final
	{
		std::cout << "Usage:\n\n"
				  << "cat <file...>\n\n"
				  << "Prints contents of files to standard output.\n"
				  << "Relative filepaths will be interpreted relative to the current working directory."
				  << std::endl;
	}

	[[nodiscard]] bool execute (int argc, char** argv) const final
	{
		if (argc < 2)
		{
			outputHelp();
			return false;
		}

		for (auto i = 1; i < argc; ++i)
		{
			File file { argv[i] };

			file.makeAbsoluteRelativeToCWD();

			if (! file.exists())
			{
				std::cerr << "File " << file.getAbsolutePath() << " does not exist!" << std::endl;
				return false;
			}

			std::cout << file << std::endl;
		}

		return true;
	}
};

}
