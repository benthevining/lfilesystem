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

/** @defgroup limes_files limes_files
	The Limes filesystem library.

	@anchor lib_limes_files

	@tableofcontents{HTML,LaTeX,XML}

	@section limes_files_overview Overview

	All classes and functions in this library are accessible after linking to the
	\c limes::lfilesystem library and including \c lfilesystem.h.

	This library provides utilities for working with the filesystem from a high-level,
	object-oriented point of view.

	@section limes_files_design Design goals

	Limes's filesystem library is built on top of the \c std::filesystem library, but I've
	preferred a more strongly object oriented design over free functions.

	I wanted a strongly-typed interface for working with filesystem objects, primarily
	to differentiate between files and directories. You can still construct paths that
	wouldn't be considered canonical for a given filesystem object type -- for instance,
	the \c Directory class won't prevent you from creating one referencing
	\c /usr/documents/a_file.txt -- but I believe that having strongly-typed classes for
	each kind of filesystem object provides a cleaner API and allows the programmer to
	more explicitly express their intent.

	If a feature is available in the standard library, I've preferred to use the standard
	library functions rather than platform-specific code wherever possible. If the standard
	expands to include features currently implemented with custom low-level platform code
	(such as the \c Volume class), then this library will be refactored to use the standard
	library code, as soon as that code is available on all platforms and compilers targeted
	by Limes.

	Another design principle of this library is that most operations are \c noexcept --
	success or failure is usually indicated by a boolean return value, or a null \c optional.
	Exceptions are only thrown when the library cannot continue in a reasonable state --
	such as in the constructor of the \c Volume class, if the volume for a certain path
	cannot be determined.

	@section limes_files_features Features

	The main entry points to this library are the \c File, \c Directory and \c SymLink classes.
	\c Volume is also a top-level class. All of these classes simply hold a path and have
	methods for manipulating it or interacting with the filesystem using this path -- none
	of these classes hold data or significant resources other than the path itself, so they
	can be freely copied around, or relatively quickly constructed and destroyed.

	This library's design also allows more custom filesystem object types to be added in
	the future -- you can inherit from \c FilesystemEntry and create your own kinds of file-like
	objects that will work with \c Directory::iterateAllChildren() and the rest of this
	library's API.

	The \c FileWatcher class is another key feature of this library: you can create an object
	to receive callbacks when a file or directory is modified.

	@section limes_files_portability Portability

	All features are designed to work as similarly across all platforms as possible. However,
	on Windows, paths containing backslashes are valid. Windows also recognizes Unix-style
	paths -- so Unix-style paths are valid on all platforms, but Windows also works with
	Windows-style paths.

	If certain features are not supported on all platforms (such as \c DynamicLibrary or
	\c FileWatcher ), functions are provided to check if the current system supports the feature.

	@section limes_files_examples Examples

	Here is some example code that demonstrates some of this library's features:

	This code listens to changes to a directory and maintains a list of all JSON files in the
	directory:

	@include files_dir_listen.cpp

	This code reads all the files in the current working directory, prints them line by line,
	concatenates all the output and prints the combined data to a new file:

	@include files_dir_enumerate.cpp


	@todo FileSearchPath class and/or glob() function

	@todo MemoryMappedFile class
 */

/** @file
	The main header for the @ref lib_limes_files "limes_files" library.

	@ingroup limes_files
 */

/** @namespace limes::files
	Filesystem utilities.

	This namespace contains all code of the @ref lib_limes_files "limes_files" library.

	@ingroup limes_files
 */

/** @example files_dir_listen.cpp
	This code listens to changes to a directory and maintains a list of all JSON files in the directory.

	@see files::FileWatcher
 */

/** @example files_dir_enumerate.cpp
	This code reads all the files in the current working directory, prints them line by line, concatenates
	all the output and prints the combined data to a new file.

	@see files::Directory
 */

#pragma once

// IWYU pragma: begin_exports
#include "./lfilesystem_CFile.h"
#include "./lfilesystem_Directory.h"
#include "./lfilesystem_DynamicLibrary.h"
#include "./lfilesystem_File.h"
#include "./lfilesystem_FilesystemEntry.h"
#include "./lfilesystem_FileWatcher.h"
#include "./lfilesystem_Misc.h"
#include "./lfilesystem_Paths.h"
#include "./lfilesystem_Permissions.h"
#include "./lfilesystem_SimpleWatcher.h"
#include "./lfilesystem_SpecialDirectories.h"
#include "./lfilesystem_SymLink.h"
#include "./lfilesystem_Volume.h"
// IWYU pragma: end_exports
