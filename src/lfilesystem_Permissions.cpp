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

#include <sstream>
#include "lfilesystem_Permissions.h"

namespace limes::files
{

Permissions::Permissions (FSPerms p) noexcept
	: perms (p)
{
}

Permissions::operator FSPerms() const noexcept
{
	return perms;
}

FSPerms Permissions::getStdPerms() const noexcept
{
	return perms;
}

Permissions& Permissions::operator= (FSPerms p) noexcept
{
	perms = p;
	return *this;
}

bool Permissions::operator== (const Permissions& other) const noexcept
{
	return perms == other.perms;
}

bool Permissions::operator== (FSPerms p) const noexcept
{
	return perms == p;
}

bool Permissions::isUnknownOrEmpty() const noexcept
{
	return perms == FSPerms::unknown || perms == FSPerms::none;
}

[[nodiscard]] static inline FSPerms perm_read_mask (Permissions::Scope s) noexcept
{
	switch (s)
	{
		case (Permissions::Scope::Owner) : return FSPerms::owner_read;
		case (Permissions::Scope::Group) : return FSPerms::group_read;
		case (Permissions::Scope::Others) : return FSPerms::others_read;
		case (Permissions::Scope::All) : return FSPerms::owner_read | FSPerms::group_read | FSPerms::others_read;
	}
}

bool Permissions::hasRead (Scope s) const noexcept
{
	return (perms & perm_read_mask (s)) != FSPerms::none;
}

Permissions Permissions::withRead (Scope s) const noexcept
{
	auto newPerms = perms;

	newPerms |= perm_read_mask (s);

	return Permissions { newPerms };
}

[[nodiscard]] static inline FSPerms perm_write_mask (Permissions::Scope s) noexcept
{
	switch (s)
	{
		case (Permissions::Scope::Owner) : return FSPerms::owner_write;
		case (Permissions::Scope::Group) : return FSPerms::group_write;
		case (Permissions::Scope::Others) : return FSPerms::others_write;
		case (Permissions::Scope::All) : return FSPerms::owner_write | FSPerms::group_write | FSPerms::others_write;
	}
}

bool Permissions::hasWrite (Scope s) const noexcept
{
	return (perms & perm_write_mask (s)) != FSPerms::none;
}

Permissions Permissions::withWrite (Scope s) const noexcept
{
	auto newPerms = perms;

	newPerms |= perm_write_mask (s);

	return Permissions { newPerms };
}

[[nodiscard]] static inline FSPerms perm_exec_mask (Permissions::Scope s) noexcept
{
	switch (s)
	{
		case (Permissions::Scope::Owner) : return FSPerms::owner_exec;
		case (Permissions::Scope::Group) : return FSPerms::group_exec;
		case (Permissions::Scope::Others) : return FSPerms::others_exec;
		case (Permissions::Scope::All) : return FSPerms::owner_exec | FSPerms::group_exec | FSPerms::others_exec;
	}
}

bool Permissions::hasExecute (Scope s) const noexcept
{
	return (perms & perm_exec_mask (s)) != FSPerms::none;
}

Permissions Permissions::withExecute (Scope s) const noexcept
{
	auto newPerms = perms;

	newPerms |= perm_exec_mask (s);

	return Permissions { newPerms };
}

bool Permissions::hasAll (Scope s) const noexcept
{
	if (s == Scope::All)
		return (perms & FSPerms::all) != FSPerms::none;

	return hasRead (s) && hasWrite (s) && hasExecute (s);
}

Permissions Permissions::withAll (Scope s) const noexcept
{
	const auto newPerms = [s, np = perms]() mutable
	{
		switch (s)
		{
			case (Scope::Owner) : np |= FSPerms::owner_all; return np;
			case (Scope::Group) : np |= FSPerms::group_all; return np;
			case (Scope::Others) : np |= FSPerms::others_all; return np;
			case (Scope::All) : return FSPerms::all;
		}
	}();

	return newPerms;
}

bool Permissions::hasStickyBit() const noexcept
{
	return (perms & FSPerms::sticky_bit) != FSPerms::none;
}

Permissions Permissions::withStickyBit() const noexcept
{
	auto newPerms = perms;

	newPerms |= FSPerms::sticky_bit;

	return Permissions { newPerms };
}

std::string Permissions::toString() const
{
	if (isUnknownOrEmpty())
		return "---------";

	std::stringstream str;

	for (auto scope : { Scope::Owner, Scope::Group, Scope::Others })
	{
		if (hasRead (scope))
			str << 'r';
		else
			str << '-';

		if (hasWrite (scope))
			str << 'w';
		else
			str << '-';

		if (hasExecute (scope))
			str << 'x';
		else
			str << '-';
	}

	return str.str();
}

Permissions Permissions::fromString (const std::string_view& string) noexcept
{
	if (string.length() != 9)
		return Permissions { FSPerms::unknown };

	Permissions newPerms;

	auto scopeStart = 0UL;

	for (auto scope : { Scope::Owner, Scope::Group, Scope::Others })
	{
		if (string[scopeStart] == 'r')
			newPerms = newPerms.withRead (scope);

		if (string[scopeStart + 1UL] == 'w')
			newPerms = newPerms.withWrite (scope);

		if (string[scopeStart + 2UL] == 'x')
			newPerms = newPerms.withExecute (scope);

		scopeStart += 3UL;
	}

	return newPerms;
}

Permissions Permissions::ownerAll() noexcept
{
	return Permissions { FSPerms::owner_all };
}

Permissions Permissions::groupAll() noexcept
{
	return Permissions { FSPerms::group_all };
}

Permissions Permissions::othersAll() noexcept
{
	return Permissions { FSPerms::others_all };
}

Permissions Permissions::all() noexcept
{
	return Permissions { FSPerms::all };
}

std::ostream& operator<< (std::ostream& os, const Permissions& value)
{
	os << value.toString();
	return os;
}

}  // namespace files
