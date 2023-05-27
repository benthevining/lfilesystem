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
#include <vector>

struct JSONWatcher final : public limes::files::FileWatcher
{
	explicit JSONWatcher (const limes::files::Directory& dir)
		: limes::files::FileWatcher (dir), directory (dir)
	{ }

	std::vector<limes::files::File> jsonFiles;
	const limes::files::Directory	directory;

	void fileCreated (const limes::files::FilesystemEntry& file) final
	{
		if (! directory.contains (file))
			return;

		if (const auto fileObj = file.getFileObject())
		{
			if (fileObj->hasFileExtension (".json"))
				jsonFiles.emplace_back (*fileObj);
		}
	}

	void fileDeleted (const limes::files::FilesystemEntry& file) final
	{
		jsonFiles.erase (std::find_if (jsonFiles.begin(), jsonFiles.end(),
									   [p = file.getAbsolutePath()] (limes::files::File& f)
									   {
										   return f.getAbsolutePath() == p;
									   }));
	}

	void fileMoved (const limes::files::FilesystemEntry& file) final
	{
		if (! directory.contains (file))
		{
			fileDeleted (file);
			return;
		}

		if (const auto fileObj = file.getFileObject())
		{
			if (! fileObj->hasFileExtension (".json"))
				return;

			const auto alreadyInList = (std::find (jsonFiles.begin(),
												   jsonFiles.end(),
												   *fileObj)
										!= jsonFiles.end());

			if (! alreadyInList)
				jsonFiles.emplace_back (*fileObj);
		}
	}
};

const limes::files::Directory dir { "/my/directory" };

JSONWatcher watcher { dir };
