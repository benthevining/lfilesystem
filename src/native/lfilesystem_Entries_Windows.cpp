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

#ifdef _MSC_VER
#	include <intrin.h>
#endif

#include <windows.h>
#include <shtypes.h>
#include <shellapi.h>
#include <cstdlib>
#include <string>
#include <array>
#include <atomic>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_Misc.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"
#include "lfilesystem/lfilesystem_File.h"
#include "lfilesystem/lfilesystem_DynamicLibrary.h"

namespace limes::files
{

bool FilesystemEntry::isHidden() const
{
	if (! exists())
		return filenameBeginsWithDot();

	DWORD atts = GetFileAttributesA (getAbsolutePath().string().data());

	return (atts & FILE_ATTRIBUTE_HIDDEN) != 0;
}

bool FilesystemEntry::moveToTrash() noexcept
{
	if (! exists())
		return false;

	auto pathStr = getAbsolutePath().string();

	// the path we pass must be double null terminated
	// pathStr += '\0';

	SHFILEOPSTRUCT fos = {};
	fos.wFunc		   = FO_DELETE;
	fos.pFrom		   = pathStr.c_str();
	fos.fFlags		   = FOF_ALLOWUNDO | FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION
			   | FOF_NOCONFIRMMKDIR | FOF_RENAMEONCOLLISION;

	return SHFileOperation (&fos) == 0;
}

#define LIMES_LOAD_WIN_DYLIB_FUNC(dll, functionName, localFunctionName, returnType, params) \
	typedef returnType (WINAPI* type##localFunctionName) params;                            \
	type##localFunctionName localFunctionName = (type##localFunctionName) dll.findFunction (#functionName)

bool FilesystemEntry::revealToUserInFileBrowser() const
{
	if (! exists())
		return false;

	DynamicLibrary dll ("Shell32.dll");

	if (! dll.isOpen())
		return false;

	LIMES_LOAD_WIN_DYLIB_FUNC (dll, ILCreateFromPathW, ilCreateFromPathW, ITEMIDLIST*, (LPCWSTR)); // cppcheck-suppress cstyleCast
	LIMES_LOAD_WIN_DYLIB_FUNC (dll, ILFree, ilFree, void, (ITEMIDLIST*) ); // cppcheck-suppress cstyleCast
	LIMES_LOAD_WIN_DYLIB_FUNC (dll, SHOpenFolderAndSelectItems, shOpenFolderAndSelectItems, HRESULT, (ITEMIDLIST*, UINT, void*, DWORD)); // cppcheck-suppress cstyleCast

	if (ilCreateFromPathW == nullptr || shOpenFolderAndSelectItems == nullptr || ilFree == nullptr)
		return false;

	if (ITEMIDLIST* const itemIDList = ilCreateFromPathW (getAbsolutePath().wstring().data()))
	{
		shOpenFolderAndSelectItems (itemIDList, 0, nullptr, 0);
		ilFree (itemIDList);
		return true;
	}

	return false;
}

#undef LIMES_LOAD_WIN_DYLIB_FUNC

LFILE_NO_EXPORT [[nodiscard]] static inline std::string getModulePathInternal (HMODULE module)
{
	std::array<wchar_t, maxPathLength()> buffer1 {};
	std::array<wchar_t, maxPathLength()> buffer2 {};

	auto* path = buffer1.data();

	auto size = GetModuleFileNameW (module, buffer1.data(), sizeof (buffer1) / sizeof (buffer1[0]));

	if (size == 0)
	{
		if (path != buffer1.data())
			std::free (path);

		return {};
	}

	if (size == static_cast<DWORD> (sizeof (buffer1) / sizeof (buffer1[0])))
	{
		auto size_ = size;

		do
		{
			if (auto* path_ = static_cast<wchar_t*> (std::realloc (path, sizeof (wchar_t) * size_ * 2)))
			{
				size_ *= 2;
				path = path_;
				size = GetModuleFileNameW (module, path, size_);
			}
			else
			{
				break;
			}
		}
		while (size == size_);

		if (size == size_)
		{
			if (path != buffer1.data())
				std::free (path);

			return {};
		}
	}

	if (! _wfullpath (buffer2.data(), path, maxPathLength()))
	{
		if (path != buffer1.data())
			std::free (path);

		return {};
	}

	if (path != buffer1.data())
		std::free (path);

	const auto length = static_cast<int> (wcslen (buffer2.data()));

	std::array<char, maxPathLength()> output {};

	auto length_ = WideCharToMultiByte (CP_UTF8, 0, buffer2.data(), length, output.data(), static_cast<int> (maxPathLength()), nullptr, nullptr);

	if (length_ == 0)
		length_ = WideCharToMultiByte (CP_UTF8, 0, buffer2.data(), length, nullptr, 0, nullptr, nullptr);

	if (length_ == 0)
		return {};

	return { output.data() };
}

namespace exec_path
{

[[nodiscard]] LFILE_EXPORT std::string get_impl()
{
	return getModulePathInternal (nullptr);
}

}  // namespace exec_path

namespace module_path
{

[[nodiscard]] LFILE_EXPORT std::string get_impl()
{
#ifdef _MSC_VER
#	define limes_get_return_address() _ReturnAddress()	 // NOLINT
#elif defined(__GNUC__)
#	define limes_get_return_address() __builtin_extract_return_addr (__builtin_return_address (0))	 // NOLINT
#endif

#ifdef limes_get_return_address

	HMODULE module;

	if (GetModuleHandleEx (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPCTSTR> (limes_get_return_address()), &module))
	{
		return getModulePathInternal (module);
	}

	return {};

#	undef limes_get_return_address

#else
	return {};
#endif
}

}  // namespace module_path

}  // namespace files
