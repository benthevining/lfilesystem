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
#include <vector>

namespace limes::files::cli::modes
{

/** TODO: options */
class CP final : public Mode
{
public:
	[[nodiscard]] std::string_view getName() const final
	{
		return "cp";
	}

	void outputHelp() const final
	{
		std::cout << "Usage:\n\n"
				  << "cp [options] <sourceFile> <targetFile>\n"
				  << "cp [options] <sourceFile...> <targetDirectory>\n\n"
				  << "In the second synopsis form, <targetDirectory> must exist."
				  << std::endl;
	}

	[[nodiscard]] bool execute (int argc, char** argv) const final
	{
		if (argc < 2)
		{
			outputHelp();
			return false;
		}

		// consume the 'cp' from the command line
		argc -= 1;
		argv += 1;

		// consume options here, adjust argc/argv accordingly

		if (argc == 2)
		{
			File source { argv[0] };
			FilesystemEntry dest { argv[1] };

			source.makeAbsoluteRelativeToCWD();
			dest.makeAbsoluteRelativeToCWD();

			if (! source.exists())
			{
				std::cerr << "Source file " << source.getAbsolutePath() << " does not exist!" << std::endl;
				return false;
			}

			if (dest.isDirectory())
				return copyFileToDirectory (source, *dest.getDirectoryObject());

			const File d { dest.getAbsolutePath() };

			return d.overwrite (source.loadAsString());
		}

		Directory dest { argv[argc - 1] };

		dest.makeAbsoluteRelativeToCWD();

		if (! dest.exists())
		{
			std::cerr << "Destination directory " << dest.getAbsolutePath() << " does not exist!" << std::endl;
			return false;
		}

		std::vector<File> sourceFiles;

		sourceFiles.reserve (argc - 1);

		for (auto i = 0; i < argc - 1; ++i)
			sourceFiles.emplace_back (argv[i]);

		for (auto& source : sourceFiles)
		{
			source.makeAbsoluteRelativeToCWD();

			if (! source.exists())
			{
				std::cerr << "Source file " << source.getAbsolutePath() << " does not exist!" << std::endl;
				return false;
			}

			if (! copyFileToDirectory (source, dest))
				;
//				return false;
		}

		return true;
	}

private:
	// TODO: this seems to always report failure?
	static bool copyFileToDirectory (const File& file, const Directory& directory)
	{
		auto dest = directory.getChildFile (file.getFilename());

		dest.replaceFileExtension (file.getFileExtension(), false);

		dest.overwrite (file.loadAsString());
	}
};

}
