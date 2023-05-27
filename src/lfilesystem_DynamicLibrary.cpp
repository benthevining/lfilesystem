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

#if defined(_WIN32) || defined(WIN32)
#	include <windows.h>
#	include <locale>  // for wstring_convert
#	include <codecvt>
#else
#	include <dlfcn.h>

#	if defined(__linux__) || defined(__gnu_linux__) || defined(linux) || defined(__linux) || defined(__EMSCRIPTEN__)
#		include <link.h>

#		if defined(__EMSCRIPTEN__)
#			include <sys/types.h>
#			include <emscripten.h>
#		else
#			include <linux/limits.h>
#		endif

#	elif defined(__APPLE__)
#		include <mach-o/dyld.h>
#		include <mach-o/nlist.h>
#		include <cstdint>
#		include <cstring>
#	endif
#endif

#include <algorithm>
#include <string>
#include "lfilesystem/lfilesystem_DynamicLibrary.h"

namespace limes::files
{

DynamicLibrary::DynamicLibrary (const std::string_view& nameOrPath) noexcept
{
	open (nameOrPath);
}

DynamicLibrary::~DynamicLibrary()
{
	try
	{
		close();
	}
	catch (...)
	{
	}
}

DynamicLibrary::DynamicLibrary (DynamicLibrary&& other) noexcept
	: handle (other.handle.load())
{
}

DynamicLibrary& DynamicLibrary::operator= (DynamicLibrary&& other) noexcept
{
	handle = other.handle.load();
	return *this;
}

bool DynamicLibrary::operator== (const DynamicLibrary& other) const noexcept
{
	if (handle.load() == other.handle.load())
		return true;

	return getFile() == other.getFile();
}

bool DynamicLibrary::operator!= (const DynamicLibrary& other) const noexcept
{
	return ! (*this == other);
}

bool DynamicLibrary::isOpen() const noexcept
{
	return handle.load() != nullptr;
}

DynamicLibrary::Handle DynamicLibrary::getHandle() const noexcept
{
	return handle.load();
}

bool DynamicLibrary::reload()
{
	if (! isOpen())
		return false;

	const auto file = getFile();

	if (! file.exists())
		return false;

	suppressNotifs = true;

	const auto result = open (file.getAbsolutePath().string());

	for (auto* l : listeners)
		l->libraryReloaded (result);

	suppressNotifs = false;

	return result;
}

[[nodiscard]] static inline std::string formatLibraryName (const std::string_view& input)
{
	auto result = std::string { input };

	// TODO: detect if input is probably a path, don't do name transforms if so

#if defined(_WIN32) || defined(WIN32)
	if (! result.ends_with (".dll"))
		result += ".dll";
#else
	if (! result.starts_with ("lib"))
		result = std::string { "lib" } + result;
#endif

	return result;
}

bool DynamicLibrary::open (const std::string_view& nameOrPath) noexcept
{
	const auto result = [this, nameOrPath]
	{
		try
		{
			close();

			if (nameOrPath.empty())
				return false;

#if defined(_WIN32) || defined(WIN32)
			const auto newHandle = ::LoadLibraryA (formatLibraryName (nameOrPath).c_str());
// #elif defined(__EMSCRIPTEN__)
// TODO: use emscripten_dlopen
#else
			const auto newHandle = ::dlopen (formatLibraryName (nameOrPath).c_str(), RTLD_LOCAL | RTLD_NOW);
#endif

			if (newHandle == nullptr)
				return false;

			handle.store (newHandle);

			return true;
		}
		catch (...)
		{
			close();
			return false;
		}
	}();

	if (! suppressNotifs)
		for (auto* l : listeners)
			l->libraryOpened (result);

	return result;
}

void DynamicLibrary::close()
{
	const auto h = handle.load();

	if (h == nullptr)
		return;

#if defined(_WIN32) || defined(WIN32)
	::FreeLibrary (h);
#else
	::dlclose (h);
#endif

	if (suppressNotifs)
		return;

	for (auto* l : listeners)
		l->libraryClosed();
}

void* DynamicLibrary::findFunction (const std::string_view& functionName) noexcept
{
	try
	{
		const auto h = handle.load();

		if (h == nullptr || functionName.empty())
			return nullptr;

#if defined(_WIN32) || defined(WIN32)
		return reinterpret_cast<void*> (::GetProcAddress (h, functionName.data()));
#else
		return ::dlsym (h, functionName.data());
#endif
	}
	catch (...)
	{
		return nullptr;
	}
}

// This takes some ugly code on Apple, which is in this file down below...
#if defined(__APPLE__)
[[nodiscard]] static const char* pathname_for_handle (void* handle) noexcept;
#endif

// TODO: Emscripten implementation
File DynamicLibrary::getFile() const
{
#ifdef __EMSCRIPTEN__
	return File {};

#else

	const auto h = handle.load();

	if (h == nullptr)
		return {};

#	if defined(_WIN32) || defined(WIN32)

	char buffer[MAX_PATH];

	::GetModuleFileNameA (h, buffer, sizeof (buffer) / sizeof (buffer[0]));

	const std::string path { buffer };

	if (path.empty())
		return {};

	return File { path };

#	elif defined(__APPLE__)

	const std::string path { pathname_for_handle (h) };

	if (path.empty())
		return {};

	return File { path };

#	else

	char buffer[PATH_MAX];

	::dlinfo (h, RTLD_DI_ORIGIN, buffer);

	return File { std::string { buffer } };

#	endif

#endif
}

std::string DynamicLibrary::getName() const
{
	auto name = getFile().getFilename (false);

	if (name.starts_with ("lib"))
		name = name.substr (2, std::string::npos);

	return name;
}

#pragma mark Ugly Apple code

// all this ugly code below is the implementation of getFile() for Apple platforms
#if defined(__APPLE__)

#	ifdef __LP64__
using mach_header_t		= mach_header_64;
using segment_command_t = segment_command_64;
using nlist_t			= nlist_64;
#	  else
using mach_header_t		= mach_header;
using segment_command_t = segment_command;
using nlist_t			= struct nlist;
#	  endif

[[nodiscard]] static inline const char* first_external_symbol_for_image (const mach_header_t* header)
{
	Dl_info info;

	if (::dladdr (header, &info) == 0)
		return nullptr;

	segment_command_t*	   seg_linkedit = nullptr;
	segment_command_t*	   seg_text		= nullptr;
	struct symtab_command* symtab		= nullptr;

	struct load_command* cmd = (struct load_command*) ((intptr_t) header + sizeof (mach_header_t));

	for (uint32_t i = 0;
		 i < header->ncmds;
		 i++, cmd = (struct load_command*) ((intptr_t) cmd + cmd->cmdsize))
	{
		switch (cmd->cmd)
		{
			case LC_SEGMENT :
				[[fallthrough]];
			case LC_SEGMENT_64 :
			{
				auto* seg_cmd = reinterpret_cast<segment_command_t*>(cmd);

				if (! std::strcmp (seg_cmd->segname, SEG_TEXT))			 // NOLINT
					seg_text = seg_cmd;									 // NOLINT
				else if (! std::strcmp (seg_cmd->segname, SEG_LINKEDIT))	 // NOLINT
					seg_linkedit = seg_cmd;								 // NOLINT
				break;
			}
			case LC_SYMTAB :
				symtab = reinterpret_cast<struct symtab_command*>(cmd);
				break;
		}
	}

	if ((seg_text == nullptr) || (seg_linkedit == nullptr) || (symtab == nullptr))
		return nullptr;

	const auto	file_slide = ((intptr_t) seg_linkedit->vmaddr - (intptr_t) seg_text->vmaddr) - seg_linkedit->fileoff;
	const auto	strings	   = (intptr_t) header + (symtab->stroff + file_slide);
	const auto* sym		   = (nlist_t*) ((intptr_t) header + (symtab->symoff + file_slide));  // NOLINT

	for (uint32_t i = 0; i < symtab->nsyms; i++, sym++)
	{
		if ((sym->n_type & N_EXT) != N_EXT || ! sym->n_value)
			continue;

		return (const char*) strings + sym->n_un.n_strx;
	}

	return nullptr;
}

[[nodiscard]] static const char* pathname_for_handle (void* handle) noexcept
{
	try
	{
		for (auto i = ::_dyld_image_count(); i > 0; --i)
		{
			if (const auto* first_symbol = first_external_symbol_for_image (reinterpret_cast<const mach_header_t*> (::_dyld_get_image_header (i))))
			{
				if (std::strlen (first_symbol) <= 1)
					continue;

				// in order to trigger findExportedSymbol instead of findExportedSymbolInImageOrDependentImages.
				// See `dlsym` implementation at http://opensource.apple.com/source/dyld/dyld-239.3/src/dyldAPIs.cpp
				// note that this is not a member function of DynamicLibrary, so this doesn't change the actual
				// held by the calling object
				handle = reinterpret_cast<void*> ((intptr_t) handle | 1);

				first_symbol++;	 // in order to remove the leading underscore

				auto* address = ::dlsym (handle, first_symbol);

				Dl_info info;

				if (::dladdr (address, &info) != 0)
					return info.dli_fname;
			}
		}

		return nullptr;
	}
	catch (...)
	{
		return nullptr;
	}
}

#endif /* __APPLE__ */

#pragma mark Reloader

DynamicLibrary::Reloader::Reloader (DynamicLibrary& libraryToReload)
	: FileWatcher (libraryToReload.getFile()), library (libraryToReload)
{
}

void DynamicLibrary::Reloader::fileDeleted (const FilesystemEntry&)
{
	library.close();
}

void DynamicLibrary::Reloader::fileModified (const FilesystemEntry&)
{
	library.reload();
}

#pragma mark Listener

DynamicLibrary::Listener::Listener (DynamicLibrary& library)
	: lib (library)
{
	lib.listeners.emplace_back (this);
}

DynamicLibrary::Listener::~Listener()
{
	lib.listeners.erase (std::remove (lib.listeners.begin(), lib.listeners.end(), this),
						 lib.listeners.end());
}

}  // namespace limes::files

namespace std
{

size_t hash<limes::files::DynamicLibrary>::operator() (const limes::files::DynamicLibrary& l) const noexcept
{
	return hash<limes::files::FilesystemEntry> {}(l.getFile());
}

}  // namespace std
