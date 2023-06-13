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

limes_get_catch2 ()

add_executable (lfilesystem_tests)

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

limes_configure_test_target (
    lfilesystem_tests
    BUNDLE_ID Limes.lfilesystem.tests
    VERSION_MAJOR "${lfilesystem_VERSION_MAJOR}"
    FULL_VERSION "${lfilesystem_VERSION}"
    TEST_PREFIX limes.files.unit.)

target_link_libraries (lfilesystem_tests PRIVATE limes::lfilesystem)

target_compile_definitions (lfilesystem_tests
                            PRIVATE "LIMES_TEST_DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data\"")

add_subdirectory (StructuredTree)
add_subdirectory (ExecLocation)
# add_subdirectory (DynamicLibrary)

# include (FindPackage/CreateTests.cmake)
