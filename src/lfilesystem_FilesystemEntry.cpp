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

#include <ctime>		  // for tm
#include <exception>	  // for exception
#include <filesystem>	  // for path, copy, operator/, absolute, cera...
#include <fstream>		  // for string, ofstream
#include <string>		  // for operator<, operator>
#include <system_error>
#include "lfilesystem/lfilesystem_Directory.h"  // for Directory
#include "lfilesystem/lfilesystem_File.h"		  // for File
#include "lfilesystem/lfilesystem_SymLink.h"	  // for SymLink
#include "lfilesystem/lfilesystem_SpecialDirectories.h"
#include "lfilesystem/lfilesystem_Volume.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"
#include "lfilesystem/lfilesystem_Misc.h"
#include "lfilesystem/lfilesystem_Paths.h"

#if defined(_WIN32) || defined(WIN32)
#	include <cctype>
#endif

/** The following functions are implemented in the platform specific sources in the native/ folder:

	FilesystemEntry::isHidden()
	FilesystemEntry::moveToTrash()
	FilesystemEntry::revealToUserInFileBrowser()
 */

namespace limes::files
{

FilesystemEntry::FilesystemEntry (const Path& pathToUse) noexcept
	: path (normalizePath (pathToUse))
{
}

FilesystemEntry& FilesystemEntry::operator= (const Path& newPath)
{
	path = normalizePath (newPath);
	return *this;
}

FilesystemEntry& FilesystemEntry::operator= (const std::string_view& newPath)
{
	return *this = Path { newPath };
}

FilesystemEntry::operator Path() const noexcept
{
	return getPath();
}

FilesystemEntry& FilesystemEntry::assignPath (const Path& newPath) noexcept
{
	path = normalizePath (newPath);
	return *this;
}

FilesystemEntry FilesystemEntry::operator/ (const std::string_view& subpathName) const
{
	return FilesystemEntry { getAbsolutePath() / subpathName };
}

FilesystemEntry& FilesystemEntry::operator/= (const std::string_view& subpathName)
{
	assignPath (getAbsolutePath() / subpathName);
	return *this;
}

FilesystemEntry& FilesystemEntry::changeName (const std::string_view& newName)
{
	path.replace_filename (newName);
	return *this;
}

static inline bool areSameIgnoringCase (const std::string_view& lhs, const std::string_view& rhs) noexcept
{
	return std::equal (lhs.begin(), lhs.end(),
					   rhs.begin(), rhs.end(),
					   [] (char a, char b)
					   {
		return std::tolower (static_cast<unsigned char> (a)) == std::tolower (static_cast<unsigned char> (b));
	});
}

bool FilesystemEntry::operator== (const FilesystemEntry& other) const noexcept
{
	std::error_code ec;

	// std::filesystem::equivalent can only be used if both paths exist
	if (exists() && other.exists())
	{
		if (isRelativePath() && other.isRelativePath())
			return std::filesystem::equivalent (path, other.path, ec);

		return std::filesystem::equivalent (getAbsolutePath(), other.getAbsolutePath(), ec);
	}

	const auto caseSensitive = [p = getAbsolutePath()]
	{
		if (const auto v = Volume::tryCreate (p))
			return v->isCaseSensitive();

		return filesystemIsCaseSensitive();
	}();

	if (caseSensitive)
	{
		if (isRelativePath() && other.isRelativePath())
			return path == other.path;

		return getAbsolutePath() == other.getAbsolutePath();
	}
	else
	{
		// case-insensitive filesystem, compare the paths as strings
		std::string thisPath, otherPath;

		if (isRelativePath() && other.isRelativePath())
		{
			thisPath  = path.string();
			otherPath = other.path.string();
		}
		else
		{
			thisPath  = getAbsolutePath().string();
			otherPath = other.getAbsolutePath().string();
		}

		return areSameIgnoringCase (thisPath, otherPath);
	}
}

bool FilesystemEntry::operator!= (const FilesystemEntry& other) const noexcept
{
	return ! (*this == other);
}

bool FilesystemEntry::operator== (const Path& other) const noexcept
{
	const FilesystemEntry e { other };
	return *this == e;
}

bool FilesystemEntry::operator!= (const Path& other) const noexcept
{
	return ! (*this == other);
}

bool FilesystemEntry::operator<(const FilesystemEntry& other) const noexcept
{
	const auto isDir	  = isDirectory();
	const auto otherIsDir = other.isDirectory();

	if (isDir && ! otherIsDir)
		return false;

	if (! isDir && otherIsDir)
		return true;

	return getAbsolutePath() < other.getAbsolutePath();
}

bool FilesystemEntry::operator> (const FilesystemEntry& other) const noexcept
{
	const auto isDir	  = isDirectory();
	const auto otherIsDir = other.isDirectory();

	if (isDir && ! otherIsDir)
		return true;

	if (! isDir && otherIsDir)
		return false;

	return getAbsolutePath() > other.getAbsolutePath();
}

bool FilesystemEntry::operator<(const Path& other) const noexcept
{
	return *this < FilesystemEntry { other };
}

bool FilesystemEntry::operator> (const Path& other) const noexcept
{
	return *this > FilesystemEntry { other };
}

Path FilesystemEntry::getPath (bool makePreferred) const noexcept
{
	if (! makePreferred)
		return normalizePath (path);

	// the make_preferred() method returns a new path
	// but also changes the one it's called on ¯\_(ツ)_/¯
	Path pathCopy { path };

	return normalizePath (pathCopy.make_preferred());
}

Path FilesystemEntry::getAbsolutePath (bool makePreferred) const noexcept
{
	// std::filesystem::absolute will return the current working directory
	// if you sent it an empty or invalid path
	if (! isValid())
		return {};

	auto abs = std::filesystem::absolute (path);

	if (! makePreferred)
		return normalizePath (abs);

	return normalizePath (abs.make_preferred());
}

std::string FilesystemEntry::getName() const noexcept
{
	return path.filename().string();
}

FilesystemEntry FilesystemEntry::getSibling (const std::string_view& siblingName) const
{
	return getDirectory().getChild (siblingName, false);
}

Directory FilesystemEntry::getDirectory() const
{
	if (isDirectory())
		return Directory { getAbsolutePath() };

	return Directory { getAbsolutePath().parent_path() };
}

Directory FilesystemEntry::getParentDirectory() const
{
	return Directory { getDirectory().getAbsolutePath().parent_path() };
}

bool FilesystemEntry::isBelow (const Directory& directory, std::size_t depthLimit) const
{
	auto dirPath = getDirectory();

	for (auto i = 0UL; i < depthLimit; ++i)
	{
		if (dirPath == directory)
			return true;

		dirPath = dirPath.getParentDirectory();
	}

	return false;
}

bool FilesystemEntry::isValid() const noexcept
{
	return isValidPath (path);
}

bool FilesystemEntry::exists() const noexcept
{
	if (! isValid())
		return false;

	return std::filesystem::exists (path);
}

FilesystemEntry::operator bool() const noexcept
{
	return exists();
}

bool FilesystemEntry::isFile() const noexcept
{
	// assume this is a file if it's not one of the other two object types
	return ! (isDirectory() || isSymLink());
}

bool FilesystemEntry::isDirectory() const noexcept
{
	// this method is virtual because this code isn't the most robust
	// particularly if this path doesn't actually exist.
	// Directory objects simply always return true here.

	const auto type = std::filesystem::status (getAbsolutePath()).type();

	return type == std::filesystem::file_type::directory;
}

bool FilesystemEntry::isSymLink() const noexcept
{
	// this method is virtual because this code isn't the most robust.
	// particularly if this path doesn't actually exist.
	// SymLink objects simply always return true here.

	const auto type = std::filesystem::status (getAbsolutePath()).type();

	return type == std::filesystem::file_type::symlink;
}

std::optional<File> FilesystemEntry::getFileObject() const noexcept
{
	if (! isValid())
		return std::nullopt;

	if (! isFile())
		return std::nullopt;

	return File { path };
}

std::optional<Directory> FilesystemEntry::getDirectoryObject() const noexcept
{
	if (! isValid())
		return std::nullopt;

	if (! isDirectory())
		return std::nullopt;

	return Directory { path };
}

std::optional<SymLink> FilesystemEntry::getSymLinkObject() const noexcept
{
	if (! isValid())
		return std::nullopt;

	if (! isSymLink())
		return std::nullopt;

	return SymLink { path };
}

bool FilesystemEntry::isAbsolutePath() const noexcept
{
	// on Windows, path.is_absolute() seems to be incorrect
	// so do some checks ourselves, for leading directory
	// separators, and for a leading drive letter
#if defined(_WIN32) || defined(WIN32)
	const auto str = path.string();

	if (str.starts_with ('/') || str.starts_with ('\\'))
		return true;

	if (str.length() >= 2)
	{
		// check for a leading drive letter
		if (str[1] == ':' && std::isupper (str[0]) != 0)
			return true;
	}
#endif

	return path.is_absolute();
}

bool FilesystemEntry::isRelativePath() const noexcept
{
	if (! isValid())
		return false;

	return path.is_relative();
}

bool FilesystemEntry::makeAbsoluteRelativeTo (const Path& basePath) noexcept
{
	if (isAbsolutePath())
		return false;

	if (! basePath.is_absolute())
		return false;

	path = basePath / path;

	return true;
}

bool FilesystemEntry::makeAbsoluteRelativeToCWD() noexcept
{
	return makeAbsoluteRelativeTo (dirs::cwd().getAbsolutePath());
}

bool FilesystemEntry::createIfDoesntExist() const noexcept
{
	if (! isValid())
		return false;

	if (exists())
		return false;

	// can't create a symlink without knowing the desired target
	// the SymLink API itself must be used to explicitly create links
	// see the static SymLink::create() functions
	if (isSymLink())
		return false;

	std::error_code ec;

	if (isDirectory())
		return std::filesystem::create_directories (getAbsolutePath(), ec);

	try
	{
		[[maybe_unused]] std::ofstream output { getAbsolutePath() };
	}
	catch(...)
	{
		return false;
	}

	return exists();
}

bool FilesystemEntry::deleteIfExists() const noexcept
{
	if (! exists() || ! isValid())
		return false;

	std::error_code ec;

	const auto filesRemoved = std::filesystem::remove_all (getAbsolutePath(), ec);

	return filesRemoved > 0 && ! exists();
}

void FilesystemEntry::touch() const
{
	if (! createIfDoesntExist())
	{
		[[maybe_unused]] std::ofstream output { getAbsolutePath() };
	}
}

bool FilesystemEntry::touch_noCreate() const
{
	if (! exists())
		return false;

	touch();
	return true;
}

std::uintmax_t FilesystemEntry::sizeInBytes() const
{
	if (! exists())
		return 0;

	// directories add up the size of all their contents recursively
	if (isDirectory())
		return getDirectoryObject()->sizeInBytes();

	return std::filesystem::file_size (path);
}

bool FilesystemEntry::setPermissions (FSPerms permissions, PermOptions options) const noexcept
{
	std::error_code ec;

	std::filesystem::permissions (getAbsolutePath(), permissions, options, ec);

	return true;
}

Permissions FilesystemEntry::getPermissions() const
{
	return Permissions { std::filesystem::status (getAbsolutePath()).permissions() };
}

FilesystemEntry::Time FilesystemEntry::getLastModificationTime() const noexcept
{
	return std::filesystem::last_write_time (getAbsolutePath());
}

bool FilesystemEntry::rename (const Path& newPath) noexcept
{
	FilesystemEntry newEntry { newPath };

	newEntry.makeAbsoluteRelativeTo (getDirectory());

	const auto pathBefore = path;

	const auto newResolvedPath = newEntry.getAbsolutePath();

	std::error_code ec;

	std::filesystem::rename (path, newResolvedPath, ec);

	path = newResolvedPath;

	return true;
}

bool FilesystemEntry::copyTo (const Path& dest, CopyOptions options) const noexcept
{
	FilesystemEntry newEntry { dest };

	newEntry.makeAbsoluteRelativeTo (getDirectory());

	std::error_code ec;

	std::filesystem::copy (getAbsolutePath(), newEntry.getAbsolutePath(), options, ec);

	return true;
}

bool FilesystemEntry::copyTo (const FilesystemEntry& dest, CopyOptions options) const noexcept
{
	return copyTo (dest.getAbsolutePath(), options);
}

std::optional<FilesystemEntry> FilesystemEntry::copyToDirectory (const Path& destDirectory, CopyOptions options) const noexcept
{
	if (! exists())
		return std::nullopt;

	Directory dir { destDirectory };

	const auto thisDir = getDirectory();

	dir.makeAbsoluteRelativeTo (thisDir);

	if (dir == thisDir)
		return std::nullopt;

	dir.createIfDoesntExist();

	const auto newPath = dir.getAbsolutePath() / getName();

	if (! copyTo (newPath, options))
		return std::nullopt;

	return FilesystemEntry { newPath };
}

bool FilesystemEntry::copyFrom (const Path& source, CopyOptions options) const noexcept
{
	FilesystemEntry sourceEntry { source };

	sourceEntry.makeAbsoluteRelativeTo (getDirectory());

	std::error_code ec;

	std::filesystem::copy (sourceEntry.getAbsolutePath(), getAbsolutePath(), options, ec);

	return true;
}

bool FilesystemEntry::copyFrom (const FilesystemEntry& source, CopyOptions options) const noexcept
{
	return copyFrom (source.getAbsolutePath(), options);
}

std::optional<Volume> FilesystemEntry::getVolume() const noexcept
{
	if (exists())
		return Volume::tryCreate (getAbsolutePath());

	return std::nullopt;
}

bool FilesystemEntry::filenameBeginsWithDot() const
{
	return getAbsolutePath().stem().string().starts_with ('.');
}

std::ostream& operator<< (std::ostream& os, const FilesystemEntry& value)
{
	os << value.getPath();
	return os;
}

}  // namespace limes::files

namespace std
{
size_t hash<limes::files::FilesystemEntry>::operator() (const limes::files::FilesystemEntry& e) const noexcept
{
	return filesystem::hash_value (e.getAbsolutePath());
}
}  // namespace std
