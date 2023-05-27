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

#include <iterator>	   // for istreambuf_iterator
#include <filesystem>  // for path, operator/
#include <fstream>	   // for string, ifstream, ofstream
#include <string>	   // for char_traits, operator+
#include <string_view>
#include <sstream>
#include <cstdio>
#include <atomic>
#include <mutex>
#include <algorithm>
#include <system_error>
#include "lfilesystem/lfilesystem_File.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Directory.h"		// for Directory
#include "lfilesystem/lfilesystem_FilesystemEntry.h"	// for Path

#ifdef __APPLE__
#	include <TargetConditionals.h>
#endif

/** The following functions are implemented in the platform specific sources in the native/ folder:

	File::getCurrentExecutable() - is in this file, but see native/ for exec_path::get_impl()
	File::getCurrentModule()     - is in this file, but see native/ for module_path::get_impl()
	File::isMacOSBundle()        - the non-OSX implementation that always returns false is in this file. See native/mac.mm for the OSX impl.
 */

namespace limes::files
{

#pragma mark   File
#pragma region File

bool File::isFile() const noexcept
{
	return true;
}

bool File::isDirectory() const noexcept
{
	return false;
}

bool File::isSymLink() const noexcept
{
	return false;
}

File& File::operator= (const Path& newPath)
{
	assignPath (newPath);
	return *this;
}

File& File::operator= (const std::string_view& newPath)
{
	assignPath (newPath);
	return *this;
}

std::string File::getFilename (bool includeExtension) const
{
	if (! includeExtension)
		return getPath().stem().string();

	return getPath().filename().string();
}

std::string File::getFileExtension() const
{
	return getPath().extension().string();
}

bool File::hasFileExtension (const std::string_view& extension) const
{
	auto ext = extension;

	if (ext[0] == '.')
		ext = ext.substr (1);

	if (ext.empty())
		return false;

	return getFileExtension().erase (0, 1) == ext;
}

bool File::hasFileExtension() const
{
	return getPath().has_extension();
}

// see native/mac.mm for the OSX implementation
#if ((! defined(__APPLE__)) || TARGET_OS_IPHONE)
bool File::isMacOSBundle() const noexcept
{
	return false;
}
#endif

bool File::replaceFileExtension (const std::string_view& newFileExtension,
								 bool					 renameOnDisk)
{
	if (renameOnDisk)
		return rename (getAbsolutePath().replace_extension (newFileExtension));

	assignPath (getAbsolutePath().replace_extension (newFileExtension));

	return false;
}

bool File::write_data ([[maybe_unused]] const char* const data,
					   [[maybe_unused]] std::size_t numBytes,
					   [[maybe_unused]] bool overwrite) const noexcept
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	if (numBytes == 0)
		return deleteIfExists();

	try
	{
		const auto mode = overwrite ? std::ios::trunc : std::ios::app;

		std::ofstream stream { getAbsolutePath().c_str(), mode };

		stream.write (data, static_cast<std::streamsize> (numBytes));

		return true;
	}
	catch(...)
	{
		return false;
	}
#endif
}

bool File::overwrite (const char* const data, std::size_t numBytes) const noexcept
{
	return write_data (data, numBytes, true);
}

bool File::overwrite (const std::string_view& text) const noexcept
{
	return overwrite (text.data(), text.size());
}

std::unique_ptr<std::ifstream> File::getInputStream() const
{
#ifdef __EMSCRIPTEN__
	return nullptr;
#else
	return std::make_unique<std::ifstream> (getAbsolutePath().c_str());
#endif
}

bool File::append (const char* const data, std::size_t numBytes) const noexcept
{
	return write_data (data, numBytes, false);
}

bool File::append (const std::string_view& text) const noexcept
{
	return write_data (text.data(), text.size(), false);
}

bool File::prepend ([[maybe_unused]] const char* const data,
					[[maybe_unused]] std::size_t numBytes) const noexcept
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	const std::string_view str { data, numBytes };

	return prepend (str);
#endif
}

bool File::prepend ([[maybe_unused]] const std::string_view& text) const noexcept
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	auto data = loadAsString();

	data = std::string { text } + data;

	return overwrite (data);
#endif
}

std::optional<File> File::duplicate() const noexcept
{
#ifdef __EMSCRIPTEN__
	return std::nullopt;
#else
	if (! exists())
		return std::nullopt;

	const auto dir = getDirectory();

	const auto newFilename = [filename = getFilename (false), extension = getFileExtension(), &dir]() -> std::string
	{
		auto newName = filename + "_copy." + extension;

		if (! dir.contains (newName))
			return newName;

		for (auto copyNum = 2; copyNum < 999; ++copyNum)
		{
			newName = filename + "_copy" + std::to_string (copyNum) + "." + extension;

			if (! dir.contains (newName))
				return newName;
		}

		return {};
	}();

	if (newFilename.empty())
		return std::nullopt;

	const File newFile { dir.getAbsolutePath() / newFilename };

	// if we chose a filename that exists, then either
	// there's an error in the filename selecting code
	// above, or this file was created between that code
	// and this assertion
	if (newFile.exists())
		return std::nullopt;

	if (! newFile.createIfDoesntExist())
		return std::nullopt;

	if (! newFile.overwrite (loadAsString()))
	{
		newFile.deleteIfExists();
		return std::nullopt;
	}

	return newFile;
#endif
}

bool File::resize ([[maybe_unused]] std::uintmax_t newSizeInBytes,
				   [[maybe_unused]] bool allowTruncation,
				   [[maybe_unused]] bool allowIncreasing) const noexcept
{
#ifdef __EMSCRIPTEN__
	return false;
#else
	if (! (allowTruncation || allowIncreasing))
		return false; // should this return true?

	if (! exists())
		return false;

	const auto initialSize = sizeInBytes();

	if (newSizeInBytes == initialSize)
		return false;

	if (initialSize > newSizeInBytes && ! allowTruncation)
		return false;

	if (initialSize < newSizeInBytes && ! allowIncreasing)
		return false;

	std::error_code ec;

	std::filesystem::resize_file (getAbsolutePath(), newSizeInBytes, ec);

	return true;
#endif
}

std::optional<File> File::createHardLink ([[maybe_unused]] const Path& path) const noexcept
{
#ifdef __EMSCRIPTEN__
	return std::nullopt;
#else
	if (! exists())
		return std::nullopt;

	File link { path };

	link.makeAbsoluteRelativeToCWD();

	std::error_code ec;

	std::filesystem::create_hard_link (getAbsolutePath(), link.getAbsolutePath(), ec);

	return link;
#endif
}

std::uintmax_t File::getHardLinkCount() const noexcept
{
	static constexpr auto error = static_cast<std::uintmax_t> (0);

	if (! exists())
		return error;

	std::error_code ec;

	return std::filesystem::hard_link_count (getAbsolutePath(), ec);
}

std::string File::loadAsString() const noexcept
{
#ifdef __EMSCRIPTEN__
	return {};
#else
	try
	{
		std::ifstream stream { getAbsolutePath().c_str() };

		using Iterator = std::istreambuf_iterator<char>;

		return { Iterator (stream), Iterator() };
	}
	catch (...)
	{
		return {};
	}
#endif
}

[[maybe_unused]] static inline std::vector<std::string> splitString (std::string_view stringToSplit,
																	 std::string_view delimiter,
																	 bool			  includeDelimiterInResults)
{
	const auto delimiterStartChar = delimiter.front();

	std::vector<std::string> tokens;

	auto tokenStart = stringToSplit.begin();
	auto pos		= tokenStart;

	while (pos != stringToSplit.end())
	{
		if (*pos == delimiterStartChar)
		{
			auto delimiterStart = pos++;

			while (pos != stringToSplit.end() && delimiter.find (*pos) != std::string_view::npos)
				++pos;

			if (pos != stringToSplit.begin())
				tokens.push_back ({ tokenStart, includeDelimiterInResults ? pos : delimiterStart });

			tokenStart = pos;
		}
		else
		{
			++pos;
		}
	}

	if (pos != stringToSplit.begin())
		tokens.push_back ({ tokenStart, pos });

	return tokens;
}

std::vector<std::string> File::loadAsLines() const
{
#ifdef __EMSCRIPTEN__
	return {};
#else
	auto tokens = splitString (loadAsString(), "\n", false);

	// if the newline char was \r\n, then strings will now end with \r
	std::transform (tokens.begin(), tokens.end(), tokens.begin(),
					[] (auto str)
					{
		if (str.ends_with ('\r'))
			return str.substr (0, str.length() - 1);

		return str;
	});

	return tokens;
#endif
}

std::unique_ptr<std::ofstream> File::getOutputStream() const
{
#ifdef __EMSCRIPTEN__
	return nullptr;
#else
	return std::make_unique<std::ofstream> (getAbsolutePath().c_str());
#endif
}

CFile File::getCfile (CFile::Mode mode) const noexcept
{
#ifdef __EMSCRIPTEN__
	return {};
#else
	if (! exists())
		return {};

	try
	{
		return CFile { getAbsolutePath(), mode };
	}
	catch (...)
	{
		return {};
	}
#endif
}

/*-------------------------------------------------------------------------------------------------------------------------*/

#pragma mark Iterator

File::Iterator File::begin() const
{
	return Iterator { loadAsLines() };
}

File::Iterator File::end() const
{
	return Iterator {};
}

File::Iterator::Iterator (VectorType&& v)
	: lines (std::make_shared<VectorType> (std::move (v)))
{
}

File::Iterator::Iterator() { }

File::Iterator& File::Iterator::operator++()
{
	++idx;
	return *this;
}

File::Iterator File::Iterator::operator++ (int)	 // NOLINT
{
	Iterator ret { *this };
	++(*this);
	return ret;
}

bool File::Iterator::operator== (Iterator other)
{
	// the end iterator is marked by a null array of lines
	if (other.lines == nullptr)
		return idx == lines->size();

	return idx == other.idx;
}

bool File::Iterator::operator!= (Iterator other)
{
	return ! (*this == other);
}

File::Iterator::reference File::Iterator::operator*() const
{
	return lines->at (idx);
}

File::Iterator::pointer File::Iterator::operator->() const
{
	return &lines->at (idx);
}

/*-------------------------------------------------------------------------------------------------------------------------*/

#pragma mark Executable path

// TODO:
// will this caching break if Limes is built as a shared library?
// maybe it's not worth the potential performance gain...

namespace exec_path
{

LFILE_NO_EXPORT static std::mutex lock;
LFILE_NO_EXPORT static bool		  checked { false };
LFILE_NO_EXPORT static Path		  result;

// defined in the platform-specific sources in the native/ directory
[[nodiscard]] std::string get_impl();

[[nodiscard]] LFILE_NO_EXPORT static inline Path get() noexcept
{
	const std::lock_guard g { lock };

	if (checked)
		return result;

	try
	{
		result	= get_impl();
		checked = true;
		return result;
	}
	catch (...)
	{
		checked = false;
		return {};
	}
}

}  // namespace exec_path

/*-------------------------------------------------------------------------------------------------------------------------*/

#pragma mark Module path

namespace module_path
{

LFILE_NO_EXPORT static std::mutex lock;
LFILE_NO_EXPORT static bool		  checked { false };
LFILE_NO_EXPORT static Path		  result;

// defined in the platform-specific sources in the native/ directory
[[nodiscard]] std::string get_impl();

[[nodiscard]] LFILE_NO_EXPORT static inline Path get() noexcept
{
	const std::lock_guard g { lock };

	if (checked)
		return result;

	try
	{
		result	= get_impl();
		checked = true;
		return result;
	}
	catch (...)
	{
		checked = false;
		return {};
	}
}

}  // namespace module_path

File File::getCurrentExecutable()
{
	return File { exec_path::get() };
}

File File::getCurrentModule()
{
	return File { module_path::get() };
}

/*-------------------------------------------------------------------------------------------------------------------------*/

#pragma mark TempFile
#pragma endregion
#pragma region TempFile

static inline Path createTmpFilepath (const Path& inputPath)
{
	if (inputPath.is_absolute())
		return inputPath;

	return dirs::temp().getAbsolutePath() / inputPath;
}

TempFile::TempFile (const Path& filepath, bool destroyOnDelete)
	: File (createTmpFilepath (filepath)), shouldDelete (destroyOnDelete)
{
	[[maybe_unused]] const auto created = createIfDoesntExist();
}

TempFile::~TempFile()
{
	if (shouldDelete)
		deleteIfExists();
}

TempFile::TempFile (TempFile&& other) noexcept
	: shouldDelete (other.shouldDelete)
{
	assignPath (other.getAbsolutePath());

	other.shouldDelete = false;
}

TempFile& TempFile::operator= (TempFile&& other) noexcept
{
	assignPath (other.getAbsolutePath());

	shouldDelete = other.shouldDelete;

	other.shouldDelete = false;

	return *this;
}

TempFile TempFile::getNextFile()
{
	static std::atomic<size_t> lastCount { 0 };

	const auto dir = dirs::temp();

	for (auto idx = lastCount.fetch_add (1UL);
		 idx < 1000UL;
		 idx = lastCount.fetch_add (1UL))
	{
		std::stringstream str;

		str << "temp_" << idx;

		const auto filename = str.str();

		if (! dir.contains (filename))
			return TempFile { filename, true };
	}

	lastCount.store (0);

	return TempFile { Path {}, false };
}

std::ostream& operator<< (std::ostream& os, const File& file)
{
	os << file.loadAsString();
	return os;
}

std::istream& operator>> (std::istream& is, const File& file)
{
	std::string content;
	is >> content;
	file.overwrite (content);
	return is;
}

}  // namespace limes::files

namespace std
{
size_t hash<limes::files::File>::operator() (const limes::files::File& f) const noexcept
{
	return hash<string> {}(f.loadAsString());
}
}  // namespace std
