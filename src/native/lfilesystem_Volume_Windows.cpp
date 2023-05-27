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
#include <pathcch.h>
#include <fileapi.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include "lfilesystem/lfilesystem_Volume.h"
#include "lfilesystem/lfilesystem_Misc.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"

namespace limes::files
{

static inline Path findRootVolumePath (const Path& inputPath)
{
	wchar_t buf[MAX_PATH] = {};

	const FilesystemEntry path { inputPath };

	if (GetVolumePathNameW (path.getAbsolutePath (true).wstring().data(),
							buf,
							static_cast<DWORD> (MAX_PATH))
		== 0)
	{
		std::stringstream stream;

		stream << "Volume path cannot be determined for path "
			   << inputPath.string();

		throw std::runtime_error { stream.str() };
	}

	Path p { buf };

	return p;
}

Volume::Volume (const Path& path)
	: rootPath (findRootVolumePath (path))
{
}

std::string Volume::getLabel() const
{
	static constexpr auto bufferSize = 64;

	char dest[bufferSize] = {};	 // NOLINT

	if (GetVolumeInformationA (rootPath.string().c_str(),
							   dest,
							   static_cast<DWORD> (bufferSize),
							   nullptr, nullptr, nullptr, nullptr, DWORD (0))
		== FALSE)
		return {};

	return { dest };
}

int Volume::getSerialNumber() const
{
	DWORD serialNum;

	if (GetVolumeInformationA (rootPath.string().c_str(),
							   nullptr, DWORD (0),
							   &serialNum,
							   nullptr, nullptr, nullptr, DWORD (0))
		== FALSE)
		return 0;

	return static_cast<int> (serialNum);
}

Volume::Type Volume::getType() const
{
	switch (GetDriveTypeA (rootPath.string().c_str()))
	{
		case (DRIVE_REMOVABLE) :
			return Type::Removable;
		case (DRIVE_FIXED) :
			return Type::HardDisk;
		case (DRIVE_REMOTE) :
			return Type::Network;
		case (DRIVE_CDROM) :
			return Type::CDRom;
		case (DRIVE_RAMDISK) :
			return Type::RAM;
		default :
			return Type::Unknown;
	}
}

bool Volume::isReadOnly() const
{
	DWORD flags;

	if (GetVolumeInformationA (rootPath.string().c_str(),
							   nullptr, DWORD (0),
							   nullptr,
							   nullptr, &flags, nullptr, DWORD (0))
		== FALSE)
		return false;

	return (flags & FILE_READ_ONLY_VOLUME) != 0;
}

bool Volume::isCaseSensitive() const
{
	DWORD flags;

	if (GetVolumeInformationA (rootPath.string().c_str(),
							   nullptr, DWORD (0),
							   nullptr,
							   nullptr, &flags, nullptr, DWORD (0))
		== FALSE)
		return filesystemIsCaseSensitive();

	return (flags & FILE_CASE_SENSITIVE_SEARCH) != 0;
}

static inline void add_volume_paths (std::vector<Volume>& volumes, PWCHAR VolumeName)
{
	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names	 = NULL;
	BOOL   Success	 = FALSE;

	for (;;)
	{
		Names = (PWCHAR) new BYTE[CharCount * sizeof (WCHAR)];

		if (! Names)
			return;

		Success = GetVolumePathNamesForVolumeNameW (
			VolumeName, Names, CharCount, &CharCount);

		if (Success)
			break;

		if (GetLastError() != ERROR_MORE_DATA)
			break;

		// TODO: could use realloc instead of deleting and re-allocating
		delete[] Names;
		Names = NULL;
	}

	if (Success)
	{
		for (auto NameIdx = Names;
			 NameIdx[0] != L'\0';
			 NameIdx += wcslen (NameIdx) + 1)
		{
			const std::wstring str { NameIdx };

			volumes.emplace_back (str);
		}
	}

	if (Names != NULL)
		delete[] Names;
}

std::vector<Volume> Volume::getAll() noexcept
{
	// see https://docs.microsoft.com/en-us/windows/win32/fileio/displaying-volume-paths

	std::vector<Volume> volumes;

	try
	{
		WCHAR VolumeName[MAX_PATH] = L"";

		auto FindHandle = FindFirstVolumeW (VolumeName, ARRAYSIZE (VolumeName));

		if (FindHandle == INVALID_HANDLE_VALUE)
			return volumes;

		WCHAR  DeviceName[MAX_PATH] = L"";
		DWORD  Error				= ERROR_SUCCESS;
		BOOL   Found				= FALSE;

		for (;;)
		{
			auto Index = wcslen (VolumeName) - 1;

			if (VolumeName[0] != L'\\' || VolumeName[1] != L'\\' || VolumeName[2] != L'?' || VolumeName[3] != L'\\' || VolumeName[Index] != L'\\')
				return volumes;

			VolumeName[Index] = L'\0';

			const auto CharCount = QueryDosDeviceW (&VolumeName[4], DeviceName, ARRAYSIZE (DeviceName));

			VolumeName[Index] = L'\\';

			if (CharCount == 0)
				return volumes;

			add_volume_paths (volumes, VolumeName);

			if (! FindNextVolumeW (FindHandle, VolumeName, ARRAYSIZE (VolumeName)))
				return volumes;
		}

		FindVolumeClose (FindHandle);
	}
	catch (...)
	{
	}

	return volumes;
}

}  // namespace files

LIMES_END_NAMESPACE
