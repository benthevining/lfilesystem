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

#include <filesystem>	 // for create_directory_symlink, create_...
#include <stdexcept>
#include <system_error>
#include "lfilesystem/lfilesystem_Directory.h"
#include "lfilesystem/lfilesystem_SymLink.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"	// for FilesystemEntry, Path

namespace limes::files
{

bool SymLink::isSymLink() const noexcept
{
	return true;
}

bool SymLink::isFile() const noexcept
{
	return false;
}

bool SymLink::isDirectory() const noexcept
{
	return false;
}

SymLink::SymLink (const Path& symLinkPath, const FilesystemEntry& linkTarget)
	: FilesystemEntry (symLinkPath)
{
	const auto l = create (getAbsolutePath(), linkTarget);

	if (! l.has_value())
		throw std::runtime_error { "Failed to create symlink" };
}

FilesystemEntry SymLink::follow (std::size_t recursionDepth) const noexcept
{
	return follow_recurse (0UL, recursionDepth);
}

FilesystemEntry SymLink::follow_recurse (std::size_t counter, std::size_t limit) const noexcept
{
	std::error_code ec;

	const FilesystemEntry target { std::filesystem::read_symlink (getAbsolutePath(), ec) };

	if (counter >= limit)
		return target;

	if (! target.isSymLink())
		return target;

	return target.getSymLinkObject()->follow_recurse (counter + 1UL, limit);
}

bool SymLink::references (const FilesystemEntry& entry, std::size_t recursionDepth) const noexcept
{
	return references_recurse (entry, 0UL, recursionDepth);
}

bool SymLink::references_recurse (const FilesystemEntry& entry, std::size_t counter, std::size_t limit) const noexcept
{
	std::error_code ec;

	const FilesystemEntry target { std::filesystem::read_symlink (getAbsolutePath(), ec) };

	if (target == entry)
		return true;

	if (counter >= limit)
		return false;

	if (! target.isSymLink())
		return false;

	return target.getSymLinkObject()->references_recurse (entry, counter + 1UL, limit);
}

// TODO: improve this?
bool SymLink::referencesSameLocationAs (const SymLink& other) const
{
	if (follow (0) == other.follow (0))
		return true;

	return follow (50) == other.follow (50);
}

bool SymLink::isDangling() const noexcept
{
	return ! follow().exists();
}

std::optional<SymLink> SymLink::create (const Path& linkPath, const FilesystemEntry& target) noexcept
{
	if (! target.exists())
		return std::nullopt;

	SymLink link { linkPath };

	link.makeAbsoluteRelativeToCWD();

	// the functions below will fail if the link path already exists as a file
	link.deleteIfExists();

	const auto targetPath = target.getAbsolutePath();

	std::error_code ec;

	if (target.isDirectory())
		std::filesystem::create_directory_symlink (targetPath, link.getAbsolutePath(), ec);
	else
		std::filesystem::create_symlink (targetPath, link.getAbsolutePath(), ec);

	return link;
}

std::optional<SymLink> SymLink::create (const Path& linkPath, const Path& target) noexcept
{
	const FilesystemEntry targetEntry { target };

	return create (linkPath, targetEntry);
}

std::optional<SymLink> SymLink::create (const Directory& newDirectory, const FilesystemEntry& target) noexcept
{
	return create (newDirectory.getAbsolutePath() / target.getAbsolutePath().filename().string(),
				   target);
}

std::optional<SymLink> SymLink::create (const Directory& newDirectory, const Path& target) noexcept
{
	const FilesystemEntry targetEntry { target };

	return create (newDirectory, targetEntry);
}

}  // namespace limes::files

namespace std
{

size_t hash<limes::files::SymLink>::operator() (const limes::files::SymLink& l) const noexcept
{
	const auto a = filesystem::hash_value (l.getAbsolutePath());
	const auto b = filesystem::hash_value (l.follow());

	// combine the hash values using the Szudzik pair method

	// TODO: deal with the -2 here...
	const auto A = a >= 0UL ? 2UL * a : -2UL * a - 1UL;	 // NOLINT
	const auto B = b >= 0UL ? 2UL * b : -2UL * b - 1UL;	 // NOLINT

	if (A >= B)
		return A * A + A + B;

	return A + B * B;
}

}  // namespace std
