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

add_executable (limes_files_tree_tester)

target_sources (limes_files_tree_tester PRIVATE main.cpp CreateTree.cmake)

limes_configure_app_bundle (
    limes_files_tree_tester BUNDLE_ID Limes.lfilesystem.treeTester
    VERSION_MAJOR "${lfilesystem_VERSION_MAJOR}" FULL_VERSION "${lfilesystem_VERSION}")

target_link_libraries (limes_files_tree_tester PRIVATE Catch2::Catch2WithMain limes::lfilesystem)

set (base_name "limes.files.StructuredTree")

set (tree_root "${CMAKE_CURRENT_BINARY_DIR}/FileTree")

add_test (NAME "${base_name}.setup" COMMAND "${CMAKE_COMMAND}" -D "TREE_ROOT=${tree_root}" -P
                                            "${CMAKE_CURRENT_LIST_DIR}/CreateTree.cmake")

set_tests_properties ("${base_name}.setup" PROPERTIES FIXTURES_SETUP LimesFilesTree)

add_test (NAME "${base_name}.cleanup" COMMAND "${CMAKE_COMMAND}" -E rm -rf "${tree_root}")

set_tests_properties ("${base_name}.cleanup" PROPERTIES FIXTURES_CLEANUP LimesFilesTree)

add_test (NAME "${base_name}.test" COMMAND limes_files_tree_tester WORKING_DIRECTORY "${tree_root}")

set_tests_properties ("${base_name}.test" PROPERTIES FIXTURES_REQUIRED LimesFilesTree)
