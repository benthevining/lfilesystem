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

#include <cstdlib>		// for getenv, size_t
#include <filesystem>	// for begin, end, operator!=, operator/
#include <type_traits>	// for conditional_t
#include <vector>		// for vector
#include <string>		// for string
#include <algorithm>
#include "lfilesystem/lfilesystem_FilesystemEntry.h"	// for FilesystemEntry, Path
#include "lfilesystem/lfilesystem_File.h"				// for File
#include "lfilesystem/lfilesystem_SymLink.h"			// for SymLink
#include "lfilesystem/lfilesystem_Misc.h"		// for PATHseparator
#include "lfilesystem/lfilesystem_Directory.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"

namespace limes::files
{

bool Directory::isDirectory() const noexcept
{
	return true;
}

bool Directory::isFile() const noexcept
{
	return false;
}

bool Directory::isSymLink() const noexcept
{
	return false;
}

Directory& Directory::operator= (const Path& newPath)
{
	assignPath (newPath);
	return *this;
}

Directory& Directory::operator= (const std::string_view& newPath)
{
	assignPath (newPath);
	return *this;
}

bool Directory::contains (const FilesystemEntry& entry, std::size_t depthLimit) const
{
	return entry.isBelow (*this, depthLimit);
}

bool Directory::contains (const std::string_view& childName) const
{
	const auto children = getAllChildren (false);

	return std::find_if (std::begin (children),
						 std::end (children),
						 [&childName] (const FilesystemEntry& e)
						 { return e.getName() == childName; })
			!= std::end (children);
}

bool Directory::createIfDoesntExist() const noexcept
{
	if (! isValid())
		return false;

	if (exists())
		return false;

	try
	{
		return std::filesystem::create_directories (getAbsolutePath());
	}
	catch(...)
	{
		return false;
	}
}

Path Directory::getRelativePath (const Path& inputPath) const
{
	return std::filesystem::relative (inputPath, getAbsolutePath());
}

bool Directory::isEmpty() const
{
	return getAllChildren().empty();
}

[[nodiscard]] static inline Path resolveChildPath (const Path& parent, const std::string_view& childName)
{
	std::string str { childName };

	while (str.starts_with ("./"))
		str = str.substr (2, std::string::npos);

#if defined(_WIN32) || defined(WIN32)
	while (str.starts_with (".\\"))
		str = str.substr (2, std::string::npos);
#endif

	return parent / str;
}

FilesystemEntry Directory::getChild (const std::string_view& childName, bool createIfNeeded) const
{
	const auto entry = FilesystemEntry { resolveChildPath (getAbsolutePath(), childName) };

	if (createIfNeeded)
		entry.createIfDoesntExist();

	return entry;
}

File Directory::getChildFile (const std::string_view& filename, bool createIfNeeded) const
{
	const auto file = File { resolveChildPath (getAbsolutePath(), filename) };

	if (createIfNeeded)
		file.createIfDoesntExist();

	return file;
}

Directory Directory::getChildDirectory (const std::string_view& subdirectoryName, bool createIfNeeded) const
{
	const auto dir = Directory { resolveChildPath (getAbsolutePath(), subdirectoryName) };

	if (createIfNeeded)
		dir.createIfDoesntExist();

	return dir;
}

SymLink Directory::createChildSymLink (const std::string_view& symLinkName,
									   const FilesystemEntry&  symLinkTarget) const
{
	return SymLink { resolveChildPath (getAbsolutePath(), symLinkName), symLinkTarget };
}

template <bool Recursive>
using IteratorType = std::conditional_t<Recursive,
										std::filesystem::recursive_directory_iterator,
										std::filesystem::directory_iterator>;

template <bool Recursive>
[[nodiscard]] static inline std::vector<FilesystemEntry> getAllDirChildren (const Path& path, bool includeHiddenFiles)
{
	std::vector<FilesystemEntry> entries;

	for (const auto& dir_entry : IteratorType<Recursive> { path })
	{
		FilesystemEntry entry { dir_entry.path() };

		if (includeHiddenFiles || ! entry.isHidden())
			entries.emplace_back (entry);
	}

	std::sort (entries.begin(), entries.end());

	return entries;
}

template <bool Recursive>
[[nodiscard]] static inline std::vector<Directory> getDirChildDirectories (const Path& path, bool includeHiddenFiles)
{
	std::vector<Directory> entries;

	for (const auto& dir_entry : IteratorType<Recursive> { path })
	{
		if (dir_entry.is_directory() && ! dir_entry.is_symlink())  // NB. without this check, the iterator will return symlinks to directories
		{
			Directory dir { dir_entry.path() };

			if (includeHiddenFiles || ! dir.isHidden())
				entries.emplace_back (dir);
		}
	}

	std::sort (entries.begin(), entries.end());

	return entries;
}

template <bool Recursive>
[[nodiscard]] static inline std::vector<File> getDirChildFiles (const Path& path, bool includeHiddenFiles)
{
	std::vector<File> entries;

	for (const auto& dir_entry : IteratorType<Recursive> { path })
	{
		if (! (dir_entry.is_directory() || dir_entry.is_symlink()))
		{
			File file { dir_entry.path() };

			if (includeHiddenFiles || ! file.isHidden())
				entries.emplace_back (file);
		}
	}

	std::sort (entries.begin(), entries.end());

	return entries;
}

template <bool Recursive>
[[nodiscard]] static inline std::vector<SymLink> getDirChildSymLinks (const Path& path, bool includeHiddenFiles)
{
	std::vector<SymLink> entries;

	for (const auto& dir_entry : IteratorType<Recursive> { path })
	{
		if (dir_entry.is_symlink())
		{
			SymLink link { dir_entry.path() };

			if (includeHiddenFiles || ! link.isHidden())
				entries.emplace_back (link);
		}
	}

	std::sort (entries.begin(), entries.end());

	return entries;
}

std::vector<File> Directory::getChildFiles (bool recurse, bool includeHiddenFiles) const
{
	if (! exists())
		return {};

	if (recurse)
		return getDirChildFiles<true> (getAbsolutePath(), includeHiddenFiles);

	return getDirChildFiles<false> (getAbsolutePath(), includeHiddenFiles);
}

void Directory::iterateFiles (FileCallback&& callback, bool recurse, bool includeHiddenFiles) const
{
	for (const auto& file : getChildFiles (recurse, includeHiddenFiles))
		callback (file);
}

bool Directory::containsSubdirectories() const
{
	if (! exists())
		return false;

	for (const auto& dir_entry : IteratorType<false> { getAbsolutePath() })
		if (dir_entry.is_directory() && ! dir_entry.is_symlink())  // NB. without this check, the iterator will return symlinks to directories
			return true;

	return false;
}

std::vector<Directory> Directory::getChildDirectories (bool recurse, bool includeHiddenFiles) const
{
	if (! exists())
		return {};

	if (recurse)
		return getDirChildDirectories<true> (getAbsolutePath(), includeHiddenFiles);

	return getDirChildDirectories<false> (getAbsolutePath(), includeHiddenFiles);
}

void Directory::iterateDirectories (DirectoryCallback&& callback, bool recurse, bool includeHiddenFiles) const
{
	for (const auto& dir : getChildDirectories (recurse, includeHiddenFiles))
		callback (dir);
}

std::vector<SymLink> Directory::getChildSymLinks (bool recurse, bool includeHiddenFiles) const
{
	if (! exists())
		return {};

	if (recurse)
		return getDirChildSymLinks<true> (getAbsolutePath(), includeHiddenFiles);

	return getDirChildSymLinks<false> (getAbsolutePath(), includeHiddenFiles);
}

void Directory::iterateSymLinks (SymLinkCallback&& callback, bool recurse, bool includeHiddenFiles) const
{
	for (const auto& link : getChildSymLinks (recurse, includeHiddenFiles))
		callback (link);
}

std::vector<FilesystemEntry> Directory::getAllChildren (bool recurse, bool includeHiddenFiles) const
{
	if (! exists())
		return {};

	if (recurse)
		return getAllDirChildren<true> (getAbsolutePath(), includeHiddenFiles);

	return getAllDirChildren<false> (getAbsolutePath(), includeHiddenFiles);
}

void Directory::iterateAllChildren (FileCallback&&		fileCallback,
									DirectoryCallback&& directoryCallback,
									SymLinkCallback&&	symLinkCallback,
									bool				recurse,
									bool				includeHiddenFiles) const
{
	for (const auto& entry : getAllChildren (recurse, includeHiddenFiles))
	{
		// we could try to optimize a bit by moving these if
		// statements outside of the loop, but for the initial
		// implementation this was the clearest to me
		if (fileCallback != nullptr)
		{
			if (const auto file = entry.getFileObject())
			{
				fileCallback (*file);
				continue;
			}
		}

		if (directoryCallback != nullptr)
		{
			if (const auto dir = entry.getDirectoryObject())
			{
				directoryCallback (*dir);
				continue;
			}
		}

		if (symLinkCallback != nullptr)
		{
			if (const auto link = entry.getSymLinkObject())
				symLinkCallback (*link);
		}
	}
}

void Directory::iterateAllChildren (FilesystemEntryCallback&& callback,
									bool					  recurse,
									bool					  includeHiddenFiles) const
{
	for (const auto& entry : getAllChildren (recurse, includeHiddenFiles))
		callback (entry);
}

std::uintmax_t Directory::sizeInBytes() const
{
	if (! exists())
		return 0;

	std::uintmax_t result = 0;

	for (const auto& child : getAllChildren (true))
		result += child.sizeInBytes();	// cppcheck-suppress useStlAlgorithm

	return result;
}

bool Directory::setAsWorkingDirectory() const
{
	return dirs::setCWD (getAbsolutePath());
}

bool Directory::isCurrentWorkingDirectory() const
{
	return *this == dirs::cwd();
}

#pragma mark Iterator

Directory::Iterator Directory::begin() const
{
	return Iterator { getAllChildren() };
}

Directory::Iterator Directory::end() const
{
	return Iterator {};
}

Directory::Iterator::Iterator (VectorType&& v)
	: entries (std::make_shared<VectorType> (std::move (v)))
{
}

Directory::Iterator::Iterator() { }

Directory::Iterator& Directory::Iterator::operator++()
{
	++idx;
	return *this;
}

Directory::Iterator Directory::Iterator::operator++ (int)  // NOLINT
{
	Iterator ret { *this };
	++(*this);
	return ret;
}

bool Directory::Iterator::operator== (Iterator other)
{
	// the end iterator is marked by a null array of child entries
	if (other.entries == nullptr)
		return idx == entries->size();

	return idx == other.idx;
}

bool Directory::Iterator::operator!= (Iterator other)
{
	return ! (*this == other);
}

Directory::Iterator::reference Directory::Iterator::operator*() const
{
	return entries->at (idx);
}

Directory::Iterator::pointer Directory::Iterator::operator->() const
{
	return &entries->at (idx);
}

}  // namespace files
