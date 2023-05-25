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

#if LIMES_WINDOWS
#	include <Windows.h>
#else
#	include <unistd.h>
#	include <sys/types.h>
#	include <pwd.h>
#	include <stdlib.h>
#endif

#include <cstdlib>
#include <vector>
#include <string>
#include <algorithm>
#include <optional>
#include "lfilesystem_Misc.h"
#include "lfilesystem_Directory.h"
#include "lfilesystem_SpecialDirectories.h"

namespace limes::files::dirs
{

Directory cwd()
{
	return Directory { std::filesystem::current_path() };
}

bool setCWD (const Path& path)
{
	const FilesystemEntry dir { path };

	if (! (dir.isAbsolutePath() && dir.exists()))
		return false;

	if (cwd() == dir)
		return false;

	std::filesystem::current_path (dir.getAbsolutePath());
	return true;
}

static const Directory startupCWD { cwd() };

Directory cwdAtStartup()
{
	return startupCWD;
}

Directory temp()
{
	return Directory { std::filesystem::temp_directory_path() };
}

static inline std::optional<std::string> getEnvironmentVariable (std::string_view variableName)
{
	if (variableName.empty())
		return std::nullopt;

	if (const auto* v = std::getenv (variableName.data()))
		return std::string { v };

#if LIMES_WINDOWS

	static constexpr auto bufSize = DWORD (1024);

	char buf[bufSize];	// NOLINT

	if (GetEnvironmentVariableA (variableName.data(), buf, bufSize) != 0)
	{
		return std::string { buf };
	}

#endif

	return std::nullopt;
}

static inline bool setEnvironmentVariable (std::string_view variableName, std::string_view newValue)
{
	if (variableName.empty())
		return false;

#if LIMES_WINDOWS

	return SetEnvironmentVariableA (variableName.data(), newValue.data()) == TRUE;

#else

	return std::setenv (variableName.data(), newValue.data(), 1) == 0;

#endif
}

[[nodiscard]] static inline std::vector<Directory> parse_PATH (std::string input)
{
	auto pos = input.find (PATHseparator());

	if (pos == std::string::npos)
		return {};

	std::vector<Directory> dirs;

	for (;
		 pos != std::string::npos;
		 pos = input.find (PATHseparator()))
	{
		dirs.emplace_back (input.substr (0, pos));
		input.erase (0, pos + 1);
	}

	if (! input.empty())
		dirs.emplace_back (input);

	for (auto& dir : dirs)
		dir.makeAbsoluteRelativeToCWD();

	return dirs;
}

std::vector<Directory> PATH()
{
	if (const auto path = getEnvironmentVariable ("PATH"))
		return parse_PATH (*path);

	return {};
}

bool appendToPATH (const Directory& dir)
{
	std::string newPATH;

	if (const auto path = getEnvironmentVariable ("PATH"))
	{
		if (const auto origPATH = parse_PATH (*path);
			std::find (origPATH.begin(), origPATH.end(), dir) != origPATH.end())
		{
			return false;
		}

		newPATH = *path;

		if (! path->ends_with (PATHseparator()))
			newPATH += PATHseparator();

		newPATH += dir.getAbsolutePath().string();
		newPATH += PATHseparator();
	}
	else
	{
		newPATH = dir.getAbsolutePath().string();
	}

	return setEnvironmentVariable ("PATH", newPATH);
}

#if LIMES_WINDOWS
Directory win_home();  // defined in SpecialDirs_Windows.cpp
#elif LIMES_ANDROID
Directory android_home();  // defined in SpecialDirs_Android.cpp
#endif

Directory home()
{
	if (const auto h = system::env::get ("HOME"))
		return Directory { *h };

#if LIMES_WINDOWS
	return win_home();
#elif LIMES_ANDROID
	return android_home();
#else
	if (auto* pw = getpwuid (getuid()))
		return Directory { Path { pw->pw_dir } };

	return {};
#endif
}

}  // namespace limes::files::dirs
