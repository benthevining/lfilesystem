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

#pragma once

#include <functional>  // for std::hash
#include <optional>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_FilesystemEntry.h"

/** @file
	This file defines the files::SymLink class.

	@ingroup limes_files
 */

namespace limes::files
{

class Directory;

/** This class represents a symbolic link on the filesystem.

	This class provides static methods for creating symbolic links, and can follow links recursively or
	non-recursively.

	@ingroup limes_files
 */
class LFILE_EXPORT SymLink final : public FilesystemEntry
{
public:
	using FilesystemEntry::FilesystemEntry;

	SymLink (const SymLink&)			= default;
	SymLink& operator= (const SymLink&) = default;

	SymLink (SymLink&&)			   = default;
	SymLink& operator= (SymLink&&) = default;

	/** Creates a symbolic link object (the link itself will also be created on the filesystem).

		Note that this constructor will always create the link itself on the filesystem if it did not exist,
		and will throw an exception if creating the link fails.

		For a non-throwing option for creating symbolic links, use the \c create() functions.

		@throws std::runtime_error An exception is thrown if the symbolic link could not be created
		correctly.

		@see create()
	 */
	explicit SymLink (const Path& symLinkPath, const FilesystemEntry& linkTarget);

	/** Follows the symbolic link to find its target.

		@param recursionDepth Controls how many successive symlinks will be followed. If this is 0,
		the first direct target of this symlink will be returned, even if it is itself a symlink.

		@todo protection from cycles? The recursion depth helps, but we could throw an exception
		if a cycle begins (one of the targets is the same as one of the previous targets). This
		would require storing the previously visited targets in memory, so the question becomes
		if this is worth it.
	 */
	[[nodiscard]] FilesystemEntry follow (std::size_t recursionDepth = 50) const noexcept;

	/** Returns true if this symbolic link references the given FilesystemEntry, either as its
		immediate target or as part of the chain of recursive symbolic links (if one exists).

		@param recursionDepth Controls how many successive symlinks will be followed. If this is 0,
		only the first direct target of this symlink will be checked, even if it is itself a symlink.
	 */
	[[nodiscard]] bool references (const FilesystemEntry& entry, std::size_t recursionDepth = 50) const noexcept;

	/** Returns true if the other symbolic link references the same target as this one, either
		as its immediate target or as part of the chain of recursive symbolic links (if one exists).
	 */
	[[nodiscard]] bool referencesSameLocationAs (const SymLink& other) const;

	/** Returns true if this symbolic link's target does not exist. */
	[[nodiscard]] bool isDangling() const noexcept;

	[[nodiscard]] bool isSymLink() const noexcept final;
	[[nodiscard]] bool isFile() const noexcept final;
	[[nodiscard]] bool isDirectory() const noexcept final;

	/** @name Symbolic link creation */
	///@{
	/** Creates a symbolic link on the filesystem.

		@param linkPath The path to the symbolic link to be created.
		@param target The filesystem object that the new link will reference.

		@returns A SymLink object referencing the newly created link; or \c std::nullopt if an error occurred.
	 */
	static std::optional<SymLink> create (const Path& linkPath, const Path& target) noexcept;
	static std::optional<SymLink> create (const Path& linkPath, const FilesystemEntry& target) noexcept;

	/** Creates a symbolic link to the specified object in the new directory.

		The created symlink will be at \c <newDirectory>/<origFilename> .

		For example, this code creates a link at the path \c /target/directory/file.xml
		that refers to the file \c /my/special/file.xml :
		@code{.cpp}
		const limes::files::Path linkTarget { "/my/special/file.xml" };

		const limes::files::Directory targetDir { "/target/directory" };

		limes::files::SymLink::create (targetDir, linkTarget);
		@endcode

		@param newDirectory The new directory to create the link in.
		@param target The filesystem object that the new link will reference.

		@returns A SymLink object referencing the newly created link; or \c std::nullopt if an error occurred.
	 */
	static std::optional<SymLink> create (const Directory& newDirectory, const Path& target) noexcept;
	static std::optional<SymLink> create (const Directory& newDirectory, const FilesystemEntry& target) noexcept;
	///@}

private:
	[[nodiscard]] FilesystemEntry follow_recurse (std::size_t counter, std::size_t limit) const noexcept;
	[[nodiscard]] bool			  references_recurse (const FilesystemEntry& entry, std::size_t counter, std::size_t limit) const noexcept;
};

}  // namespace limes::files

namespace std
{

/** A specialization of \c std::hash for SymLink objects.
	The hash value is computed using both the link's path and the path of the target object.

	@ingroup limes_files
 */
template <>
struct LFILE_EXPORT hash<limes::files::SymLink> final
{
	size_t operator() (const limes::files::SymLink& l) const noexcept;
};

}  // namespace std
