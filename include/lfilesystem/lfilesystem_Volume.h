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

#include <string>
#include <cstdint>
#include <filesystem>
#include <vector>
#include <optional>
#include <ostream>
#include "lfilesystem/lfilesystem_Export.h"
#include "lfilesystem/lfilesystem_SpecialDirectories.h"

/** @file
	This file defines the files::Volume class.

	@ingroup limes_files
 */

namespace limes::files
{

using Path = std::filesystem::path;

class FilesystemEntry;

/** This class represents a logical filesystem volume.

	You can construct a Volume object from any path, and the resulting Volume object
	will refer to the logical volume where the passed path was mounted at the time
	of the Volume object's construction.

	@internal
	A volume is internally stored using its root path, just like any other filesystem
	object, but this class does not implement the FilesystemEntry interface because
	obviously volumes themselves do not support any of the manipulation functions that
	FilesystemEntry provides.
	@endinternal

	@ingroup limes_files

	@todo ConstructionError custom exception type? This could allow more specific
	error checking. The error could contain the original passed path.
	@todo supportsHardLinks()?
 */
class LFILE_EXPORT Volume final
{
public:
	/** Creates a Volume object representing the volume that the passed path exists on.

		@throws std::runtime_error An exception will be thrown if the volume cannot be
		determined for the passed path.

		@see tryCreate()
	 */
	explicit Volume (const Path& path = dirs::cwd().getAbsolutePath());

	/** A \c noexcept method for creating a volume object. This will return a null optional
		if the volume cannot be correctly determined for the passed path.
	 */
	[[nodiscard]] static std::optional<Volume> tryCreate (const Path& path = dirs::cwd().getAbsolutePath()) noexcept;

	/** @name Equality comparison
		Compares the root paths of two volumes for equality.
	 */
	///@{
	bool operator== (const Volume& other) const noexcept;

	/** @todo test coverage */
	bool operator!= (const Volume& other) const noexcept;
	///@}

	/** Returns true if this volume contains the passed FilesystemEntry.
		If the volume of the passed FilesystemEntry cannot be determined, this function
		will return false by default.
	 */
	[[nodiscard]] bool contains (const FilesystemEntry& file) const noexcept;

	/** Returns this volume's label.
		This will always return an empty string on iOS/watchOS/tvOS.

		@see volume::label()

		@todo test coverage
	 */
	[[nodiscard]] std::string getLabel() const;

	/** Returns this volume's serial number.
		If the serial number cannot be retrieved, this will return 0.
		This will always return 0 on Apple systems.
		Note than on Linux systems, this may fail (and return 0) if the calling process
		does not have root privileges.

		@see volume::serialNumber()

		@todo test coverage
	 */
	[[nodiscard]] int getSerialNumber() const;

	/** Returns the number of available bytes on this volume.

		@see volume::bytesFree()
	 */
	[[nodiscard]] std::uintmax_t bytesFree() const;

	/** Returns the total size of this volume in bytes.

		@see volume::totalBytes()
	 */
	[[nodiscard]] std::uintmax_t totalBytes() const;

	/** Returns the root path of this volume.
		On Windows, this might be a drive letter such as \c C:\ or \c D:\ ; on Linux this
		might be something like \c /media/usb/drive-1/ .
	 */
	[[nodiscard]] Path getPath() const noexcept;

	/** Represents the type of a filesystem volume. */
	enum class Type
	{
		CDRom,		///< Indicates this volume is a CD.
		HardDisk,	///< Indicates this volume is on the computer's hard disk.
		Removable,	///< Indicates this volume is a removable drive, such as a USB drive, etc.
		Network,	///< Indicates this volume is a remote/network drive.
		RAM,		///< Indicates this volume is a RAM drive.
		Unknown		///< The type of this volume cannot be determined.
	};

	/** Returns the type of this volume.

		@see volume::type()

		@todo Linux: detect RAM type

		@todo Mac: detect RAM & Network types
	 */
	[[nodiscard]] Type getType() const;

	/** Returns true if this volume is read-only.

		@see volume::readOnly()
	 */
	[[nodiscard]] bool isReadOnly() const;

	/** Returns true if paths on this volume are case-sensitive.

		@see volume::caseSensitive()
	 */
	[[nodiscard]] bool isCaseSensitive() const;

	/** Returns all currently mounted volumes.
		Currently mounted volumes are rescanned every time you call this function.
	 */
	[[nodiscard]] static std::vector<Volume> getAll() noexcept;

private:
	Path rootPath;
};

/** Writes the Volume object's path to the output stream.

	@ingroup limes_files
	@relates Volume
 */
std::ostream& operator<< (std::ostream& os, const Volume& value);

/** This namespace contains free functions for easily querying properties of the volume that
	the current working directory is mounted on. The advantage of these functions is that if
	the current volume cannot be successfully queried, they fall back to sensible defaults.
	If you need to know whether the actual volume can be queried, you can call
	\c Volume::tryCreate() .

	@ingroup limes_files
 */
namespace volume
{

/** Returns the current volume's label.

	@see Volume::getLabel()
 */
[[nodiscard]] LFILE_EXPORT std::string label();

/** Returns the current volume's serial number.

	@see Volume::getSerialNumber()
 */
[[nodiscard]] LFILE_EXPORT int serialNumber();

/** Returns the number of bytes free on the current volume.

	@see Volume::bytesFree()
 */
[[nodiscard]] LFILE_EXPORT std::uintmax_t bytesFree();

/** Returns the total number of bytes on the current volume.

	@see Volume::totalBytes()
 */
[[nodiscard]] LFILE_EXPORT std::uintmax_t totalBytes();

/** Returns the type of the current volume.

	@see Volume::getType()
 */
[[nodiscard]] LFILE_EXPORT Volume::Type type();

/** Returns true if the current volume is mounted as read-only.

	@see Volume::isReadOnly()
 */
[[nodiscard]] LFILE_EXPORT bool readOnly();

/** Returns true if the current volume is case-sensitive.

	@see Volume::isCaseSensitive(), filesystemIsCaseSensitive()
 */
[[nodiscard]] LFILE_EXPORT bool caseSensitive();

}  // namespace volume

}  // namespace files
