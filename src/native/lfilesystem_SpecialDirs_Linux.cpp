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

#if ! LIMES_LINUX
#	error
#endif

#include <algorithm>
#include <cctype>
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Directory.h"
#include "lfilesystem/lfilesystem_File.h"

namespace limes::files::dirs
{

Directory commonAppData()
{
	return Directory { "/opt" };
}

Directory commonDocuments()
{
	return Directory { "/opt" };
}

Directory apps()
{
	return Directory { "/usr" };
}

static inline void trim_string (std::string& string)
{
	if (string.empty())
		return;

	// trim start
	string.erase (string.begin(), std::find_if (string.begin(), string.end(), [] (unsigned char ch)
												{ return ! std::isspace (ch); }));

	// trim end
	string.erase (std::find_if (string.rbegin(), string.rend(), [] (unsigned char ch)
								{ return ! std::isspace (ch); })
					  .base(),
				  string.end());
}

static inline std::string replaceInString (std::string input, std::string_view stringToReplace, std::string_view replaceWith)
{
	std::size_t pos = 0UL;

	while ((pos = input.find (stringToReplace, pos)) != std::string::npos)
	{
		input.erase (pos, stringToReplace.length());
		input.insert (pos, replaceWith);
		pos += replaceWith.length();
	}

	return input;
}

static inline std::string fromFirstOccurrenceOf (std::string input, std::string_view stringToFind)
{
	const auto pos = input.find (stringToFind);

	if (pos == std::string::npos)
		return input;

	return input.substr (pos + stringToFind.length(), input.length());
}

static inline std::string unquotedString (std::string string)
{
	auto dropFirstChars = [&string](std::size_t numChars)
	{ 
		string = string.substr (numChars, string.length());
	};

	if (string.starts_with ("\\\""))
		dropFirstChars (2UL);
	else if (string.starts_with ('"'))
		dropFirstChars (1UL);
	else if (string.starts_with ("\\\'"))
		dropFirstChars (2UL);
	else if (string.starts_with ('\''))
		dropFirstChars (1UL);

	auto dropLastChars = [&string](std::size_t numChars)
	{
		for (auto i = 0UL; i < numChars; ++i)
		{
			if (string.empty())
				return;

			string.pop_back();
		}
	};

	if (string.ends_with ("\\\""))
		dropLastChars (2UL);
	else if (string.ends_with ('"'))
		dropLastChars (1UL);
	else if (string.ends_with ("\\\'"))
		dropLastChars (2UL);
	else if (string.ends_with ('\''))
		dropLastChars (1UL);

	return string;
}

// TODO: could these results be cached?
static inline Directory resolveXDGFolder (const char* const type, const char* const fallbackFolder)
{
	for (auto& line : File { "~/.config/user-dirs.dirs" }.loadAsLines())
	{
		trim_string (line);

		if (line.starts_with (type))
		{
			auto path = replaceInString (line, "$HOME", home().getAbsolutePath().string());

			path = fromFirstOccurrenceOf (path, "=");

			trim_string (path);

			const Directory d { Path { unquotedString (path) } };

			if (d.exists())
				return d;
		}
	}

	return Directory { fallbackFolder };
}

Directory desktop()
{
	return resolveXDGFolder ("XDG_DESKTOP_DIR", "~/Desktop");
}

Directory userDocuments()
{
	return resolveXDGFolder ("XDG_DOCUMENTS_DIR", "~/Documents");
}

Directory userAppData()
{
	return resolveXDGFolder ("XDG_CONFIG_HOME", "~/.config");
}

Directory downloads()
{
	return resolveXDGFolder ("XDG_DOWNLOAD_DIR", "~/Downloads");
}

}  // namespace limes::files::dirs
