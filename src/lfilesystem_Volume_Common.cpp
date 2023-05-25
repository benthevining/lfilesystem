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

#include <limes_namespace.h>
#include <limes_platform.h>
#include <filesystem>
#include <optional>
#include "./Volume.h"
#include "../entries/FilesystemEntry.h"
#include "../misc/misc.h"

namespace limes::files
{

std::optional<Volume> Volume::tryCreate (const Path& path) noexcept
{
	try
	{
		return Volume { path };
	}
	catch (...)
	{
		return std::nullopt;
	}
}

std::uintmax_t Volume::bytesFree() const
{
	return std::filesystem::space (rootPath).free;
}

std::uintmax_t Volume::totalBytes() const
{
	return std::filesystem::space (rootPath).capacity;
}

Path Volume::getPath() const noexcept
{
	return rootPath;
}

bool Volume::operator== (const Volume& other) const noexcept
{
	// should we use std::filesystem::equivalent() here??
	return rootPath == other.rootPath;
}

bool Volume::operator!= (const Volume& other) const noexcept
{
	return rootPath != other.rootPath;
}

bool Volume::contains (const FilesystemEntry& file) const noexcept
{
	try
	{
		const Volume other { file.getAbsolutePath() };

		return *this == other;
	}
	catch (...)
	{
		return false;
	}
}

std::ostream& operator<< (std::ostream& os, const Volume& value)
{
	os << value.getPath();
	return os;
}

namespace volume
{

std::string label()
{
	if (const auto volume = Volume::tryCreate())
		return volume->getLabel();

	return {};
}

int serialNumber()
{
	if (const auto volume = Volume::tryCreate())
		return volume->getSerialNumber();

	return 0;
}

std::uintmax_t bytesFree()
{
	if (const auto volume = Volume::tryCreate())
		return volume->bytesFree();

	return 0;
}

std::uintmax_t totalBytes()
{
	if (const auto volume = Volume::tryCreate())
		return volume->totalBytes();

	return 0;
}

Volume::Type type()
{
	if (const auto volume = Volume::tryCreate())
		return volume->getType();

	return Volume::Type::Unknown;
}

bool readOnly()
{
	if (const auto volume = Volume::tryCreate())
		return volume->isReadOnly();

	return false;
}

bool caseSensitive()
{
	if (const auto volume = Volume::tryCreate())
		return volume->isCaseSensitive();

	return filesystemIsCaseSensitive();
}

}  // namespace volume

}  // namespace files
