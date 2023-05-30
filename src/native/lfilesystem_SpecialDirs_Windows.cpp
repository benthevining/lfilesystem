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

#include <windows.h>
#include <shtypes.h>
#include <Knownfolders.h>
#include <shlobj_core.h>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Directory.h"

namespace limes::files::dirs
{

[[nodiscard]] static inline Directory getWinFolderPath (REFKNOWNFOLDERID folder)
{
	wchar_t* p { nullptr };

	if (SUCCEEDED (SHGetKnownFolderPath (folder, 0, NULL, &p)))
	{
		const auto path = Path { p };
		CoTaskMemFree (p);
		return Directory { path };
	}

	CoTaskMemFree (p);
	return {};
}

[[nodiscard]] LFILE_EXPORT Directory win_home()
{
	return getWinFolderPath (FOLDERID_Profile);
}

Directory commonAppData()
{
	return getWinFolderPath (FOLDERID_ProgramData);
}

Directory commonDocuments()
{
	return getWinFolderPath (FOLDERID_PublicDocuments);
}

Directory apps()
{
	// TODO - ??
	//#if LIMES_32BIT
	//	return getWinFolderPath (FOLDERID_ProgramFiles);
	//#else
	return getWinFolderPath (FOLDERID_ProgramFilesX86);
	//#endif
}

Directory desktop()
{
	return getWinFolderPath (FOLDERID_Desktop);
}

Directory userDocuments()
{
	return getWinFolderPath (FOLDERID_Documents);
}

Directory userAppData()
{
	return getWinFolderPath (FOLDERID_RoamingAppData);
}

Directory downloads()
{
	return getWinFolderPath (FOLDERID_Downloads);
}

}  // namespace limes::files::dirs
