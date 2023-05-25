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
#include "lfilesystem/lfilesystem_File.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Directory.h"		// for Directory
#include "lfilesystem/lfilesystem_FilesystemEntry.h"	// for Path

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
#if ! LIMES_OSX
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

bool File::write_data (const char* const data, std::size_t numBytes, bool overwrite) const noexcept
{
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
}

bool File::overwrite (const char* const data, std::size_t numBytes) const noexcept
{
	return write_data (data, numBytes, true);
}

bool File::overwrite (const memory::RawData& data) const noexcept
{
	return write_data (data.getData(), data.getSize(), true);
}

bool File::overwrite (const std::string_view& text) const noexcept
{
	return overwrite (text.data(), text.size());
}

std::unique_ptr<std::ifstream> File::getInputStream() const
{
	return std::make_unique<std::ifstream> (getAbsolutePath().c_str());
}

bool File::append (const char* const data, std::size_t numBytes) const noexcept
{
	return write_data (data, numBytes, false);
}

bool File::append (const memory::RawData& data) const noexcept
{
	return write_data (data.getData(), data.getSize(), false);
}

bool File::append (const std::string_view& text) const noexcept
{
	return write_data (text.data(), text.size(), false);
}

bool File::prepend (const memory::RawData& data) const noexcept
{
	return prepend (data.getData(), data.getSize());
}

bool File::prepend (const char* const data, std::size_t numBytes) const noexcept
{
	auto dataObj = loadAsData();

	dataObj.append (data, numBytes);

	return overwrite (dataObj);
}

bool File::prepend (const std::string_view& text) const noexcept
{
	auto data = loadAsString();

	data = std::string { text } + data;

	return overwrite (data);
}

std::optional<File> File::duplicate() const noexcept
{
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
	LIMES_ASSERT (! newFile.exists());

	if (! newFile.createIfDoesntExist())
		return std::nullopt;

	if (! newFile.overwrite (loadAsData()))
	{
		newFile.deleteIfExists();
		return std::nullopt;
	}

	return newFile;
}

bool File::resize (std::uintmax_t newSizeInBytes, bool allowTruncation, bool allowIncreasing) const noexcept
{
	// why call this function if you don't want to change the file size?
	LIMES_ASSERT (allowTruncation || allowIncreasing);

	if (! exists())
		return false;

	const auto initialSize = sizeInBytes();

	if (newSizeInBytes == initialSize)
		return false;

	if (initialSize > newSizeInBytes && ! allowTruncation)
		return false;

	if (initialSize < newSizeInBytes && ! allowIncreasing)
		return false;

	try
	{
		std::filesystem::resize_file (getAbsolutePath(), newSizeInBytes);
		return true;
	}
	catch (...)
	{
		return false;
	}
}

std::optional<File> File::createHardLink (const Path& path) const
{
	if (! exists())
		return std::nullopt;

	try
	{
		File link { path };

		link.makeAbsoluteRelativeToCWD();

		std::filesystem::create_hard_link (getAbsolutePath(), link.getAbsolutePath());

		return link;
	}
	catch (...)
	{
		return std::nullopt;
	}
}

std::uintmax_t File::getHardLinkCount() const noexcept
{
	static constexpr auto error = static_cast<std::uintmax_t> (0);

	if (! exists())
		return error;

	try
	{
		return std::filesystem::hard_link_count (getAbsolutePath());
	}
	catch (...)
	{
		return error;
	}
}

memory::RawData File::loadAsData() const noexcept
{
	try
	{
		std::ifstream stream { getAbsolutePath().c_str() };

		return memory::RawData { stream };
	}
	catch (...)
	{
		return {};
	}
}

std::string File::loadAsString() const noexcept
{
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
}

std::vector<std::string> File::loadAsLines() const
{
	return str::splitAtNewlines (loadAsString());
}

std::unique_ptr<std::ofstream> File::getOutputStream() const
{
	return std::make_unique<std::ofstream> (getAbsolutePath().c_str());
}

CFile File::getCfile (CFile::Mode mode) const noexcept
{
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

	// you've created a TempFile referring to a filepath that
	// already existed!
	LIMES_ASSERT (created);
}

TempFile::~TempFile()
{
	if (shouldDelete)
	{
		try
		{
			deleteIfExists();
		}
		catch (...)
		{
		}
	}
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

	LIMES_ASSERT (false);
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
