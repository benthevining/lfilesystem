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

#include <filesystem>
#include <string>
#include <string_view>
#include <ostream>
#include "lfilesystem/lfilesystem_Export.h"

/** @file
	This file defines the Permissions class.
	@ingroup limes_files
 */

namespace limes::files
{

/** A typedef for the standard library permissions type.
	@ingroup limes_files
 */
using FSPerms = std::filesystem::perms;

/** This class encapsulates the standard library's permissions bitmask type,
	and provides some higher-level object oriented features for working with
	permissions.

	This class is incredibly light, as it holds only a single integral value,
	so don't be afraid to copy them around.

	@ingroup limes_files
 */
class LFILE_EXPORT Permissions final
{
public:
	/** @name Constructors */
	///@{

	/** Creates a Permissions object holding the value \c FSPerms::none . */
	Permissions() = default;

	/** Creates a Permissions object holding the specified permissions value. */
	Permissions (FSPerms p) noexcept;  // cppcheck-suppress noExplicitConstructor

	///@}

	/** @name Conversion to standard permissions type */
	///@{

	/** Implicitly converts this Permissions object to the standard library permissions
		bitmask it holds.

		@see getStdPerms()
	 */
	operator FSPerms() const noexcept;

	/** Returns the standard library permissions bitmask this object holds.
		@todo test coverage
	 */
	[[nodiscard]] FSPerms getStdPerms() const noexcept;

	///@}

	Permissions (const Permissions&)			= default;
	Permissions& operator= (const Permissions&) = default;

	Permissions (Permissions&&)			   = default;
	Permissions& operator= (Permissions&&) = default;

	/** Assigns a new permissions bitmask to this object. */
	Permissions& operator= (FSPerms p) noexcept;

	/** @name Equality comparisons */
	///@{

	/** Compares permissions for equality. */
	bool operator== (const Permissions& other) const noexcept;
	bool operator== (FSPerms p) const noexcept;	 /// @todo test coverage

	///@}

	/** This enum represents a scope of permissions.

		Permissions may be scoped to the file's owner, that user's group, all other users,
		or all computer users.
	 */
	enum class LFILE_EXPORT Scope
	{
		Owner,	 ///< This scope includes only the owner of the file.
		Group,	 ///< This scope includes the file's user group.
		Others,	 ///< This scope includes all users outside the file's user group.
		All		 ///< This scope includes all computer users.
	};

	/** @name Permissions queries */
	///@{

	/** Returns true if this permissions object holds the values \c FSPerms::none or
		\c FSPerms::unknown .
	 */
	bool isUnknownOrEmpty() const noexcept;

	/** Returns true if these permissions include read access for the specified scope.
		@see withRead()
	 */
	bool hasRead (Scope s = Scope::All) const noexcept;

	/** Returns true if these permissions include write access for the specified scope.
		@see withWrite()
	 */
	bool hasWrite (Scope s = Scope::All) const noexcept;

	/** Returns true if these permissions include execution access for the specified scope.
		@see withExecute()
	 */
	bool hasExecute (Scope s = Scope::All) const noexcept;

	/** Returns true if these permissions include read, write, and execution access for the
		specified scope.

		@see withAll()
	 */
	bool hasAll (Scope s = Scope::All) const noexcept;

	/** Returns true if these permissions have the sticky bit set.

		This bit's effect is implementation-defined, but POSIX XSI specifies that when set
		on a directory, only file owners may delete files even if the directory is writeable
		to others (used with \c /tmp ).

		@see withStickyBit()

		@todo test coverage
	 */
	bool hasStickyBit() const noexcept;

	///@}

	/** @name Adding permissions */
	///@{

	/** Returns a new permissions object that adds read access in the specified scope to this
		object's existing permissions.

		@see hasRead()
	 */
	[[nodiscard]] Permissions withRead (Scope s = Scope::All) const noexcept;

	/** Returns a new permissions object that adds write access in the specified scope to this
		object's existing permissions.

		@see hasWrite()
	 */
	[[nodiscard]] Permissions withWrite (Scope s = Scope::All) const noexcept;

	/** Returns a new permissions object that adds execution access in the specified scope to
		this object's existing permissions.

		@see hasExecute()
	 */
	[[nodiscard]] Permissions withExecute (Scope s = Scope::All) const noexcept;

	/** Returns a new permissions object that adds read, write, and execution access in the
		specified scope to this object's existing permissions.

		@see hasAll()

		@todo test coverage
	 */
	[[nodiscard]] Permissions withAll (Scope s = Scope::All) const noexcept;

	/** Returns a new permissions object that adds the sticky bit to this object's existing
		permissions.

		This bit's effect is implementation-defined, but POSIX XSI specifies that when set
		on a directory, only file owners may delete files even if the directory is writeable
		to others (used with \c /tmp ).

		@see hasStickyBit()

		@todo test coverage
	 */
	[[nodiscard]] Permissions withStickyBit() const noexcept;

	///@}

	/** Returns a string representation of these permissions of the form \c rwxrwxrwx , where
		the first three characters represent the \c owner scope, the next three the \c group
		scope, and the final three the \c others scope. Each set of three characters consists
		of \c rwx , representing "read", "write", and "execute", and these characters will be
		replaced with a \c - if that permission is not present scope.

		For example, if the only permissions set are read and write permission for the file
		owner, this function will return \c rw------- . If all other users can read the file,
		but not write to it, this function will return \c rw-r--r-- . A file with all permissions
		set globally will return \c rwxrwxrwx .

		@see fromString()

		@todo test coverage for unknown/empty case
	 */
	[[nodiscard]] std::string toString() const;

	/** This function parses a string in the format returned by \c toString() . See that function
		for a detailed description of the input string.

		If the input string is not 9 characters long, this will return a Permissions object holding
		the \c unknown value. For each set of three characters, this function simply checks for the
		presence of r, w, and x characters in the expected positions; other incorrect or invalid
		inputs will not throw errors.

		You can check if this function succeeded by calling \c isUnknownOrEmpty() on the returned
		Permissions object.

		@see toString()
	 */
	[[nodiscard]] static Permissions fromString (const std::string_view& string) noexcept;

	/** Returns a Permissions object with read, write, and execution access for the file owner. */
	[[nodiscard]] static Permissions ownerAll() noexcept;

	/** Returns a Permissions object with read, write, and execution access for the file owner's
		user group.
	 */
	[[nodiscard]] static Permissions groupAll() noexcept;

	/** Returns a Permissions object with read, write, and execution access for all users not in the
		file owner's user group.

		@todo test coverage
	 */
	[[nodiscard]] static Permissions othersAll() noexcept;

	/** Returns a Permissions object with read, write, and execution access for all computer users.
	 */
	[[nodiscard]] static Permissions all() noexcept;

private:
	FSPerms perms { FSPerms::none };
};

/** Writes a Permissions object's string representation to the output stream.

	@ingroup limes_files
	@relates Permissions
 */
LFILE_EXPORT std::ostream& operator<< (std::ostream& os, const Permissions& value);

}  // namespace limes::files
