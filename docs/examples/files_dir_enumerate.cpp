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
#include <sstream>
#include <iostream>

void iterate_directory()
{
	const auto dir = limes::files::dirs::cwd();

	std::stringstream stream;

	dir.iterateFiles ([&stream] (const limes::files::File& f)
					  {
						for (const auto& line : f.loadAsLines())
						{
							std::cout << line << std::endl;
							stream << line;
						} });

	const auto newFile = dir.getChildFile ("concat.txt");

	newFile.overwrite (stream.str());
}
