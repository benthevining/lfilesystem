# ======================================================================================
#  __    ____  __  __  ____  ___
# (  )  (_  _)(  \/  )( ___)/ __)
#  )(__  _)(_  )    (  )__) \__ \
# (____)(____)(_/\/\_)(____)(___/
#
#  This file is part of the Limes open source library and is licensed under the terms of the GNU Public License.
#
#  Commercial licenses are available; contact the maintainers at ben.the.vining@gmail.com to inquire for details.
#
# ======================================================================================

include (FetchContent)

FetchContent_Declare (
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG origin/devel
	GIT_SHALLOW ON
	FIND_PACKAGE_ARGS 3 GLOBAL)

FetchContent_MakeAvailable (Catch2)

if (catch2_SOURCE_DIR)
	list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif ()

include (Catch)

add_executable (lfilesystem_tests)

set_target_properties (lfilesystem_tests PROPERTIES MACOSX_BUNDLE OFF)

target_sources (
	lfilesystem_tests
	PRIVATE CFile.cpp
			Directory.cpp
			File.cpp
			FilesystemEntry.cpp
			FileWatcher.cpp
			PathFunctions.cpp
			Permissions.cpp
			SpecialDirs.cpp
			SymLink.cpp
			Volume.cpp)

target_link_libraries (lfilesystem_tests PRIVATE Catch2::Catch2WithMain limes::lfilesystem)

target_compile_definitions (lfilesystem_tests
							PRIVATE "LIMES_TEST_DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data\"")

catch_discover_tests (
	lfilesystem_tests
	EXTRA_ARGS
	--warn
	NoAssertions
	--order
	rand
	--verbosity
	high
	TEST_PREFIX
	"Limes.files.unit.")

if (NOT EMSCRIPTEN)
	add_subdirectory (StructuredTree)
	add_subdirectory (ExecLocation)
	# add_subdirectory (DynamicLibrary)
endif ()

include (FindPackage/CreateTests.cmake)
