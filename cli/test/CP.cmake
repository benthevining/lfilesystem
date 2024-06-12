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

include_guard (GLOBAL)

set (base_name "limes.files.cli.cp")

#

set (dest_file "${CMAKE_CURRENT_BINARY_DIR}/copied.cmake")

file (REMOVE "${dest_file}")

add_test (NAME "${base_name}.simple-copy" COMMAND limes::lfile cp "${CMAKE_CURRENT_LIST_FILE}"
                                                  "${dest_file}")

#

set (dest_dir "${CMAKE_CURRENT_BINARY_DIR}/copy-test-dest-directory")

file (REMOVE_RECURSE "${dest_dir}")

add_test (NAME "${base_name}.nonexist-dest-dir"
          COMMAND limes::lfile cp "${CMAKE_CURRENT_LIST_FILE}" "${CMAKE_PARENT_LIST_FILE}"
                  "${dest_dir}")

set_tests_properties (
    "${base_name}.nonexist-dest-dir"
    PROPERTIES PASS_REGULAR_EXPRESSION "Destination directory \"${dest_dir}\" does not exist!")
