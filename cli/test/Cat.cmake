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

set (
    test_file_content
    [[
This is a test file generated by CMake to be read by the cat command-line tool.
]])

file (WRITE "${CMAKE_CURRENT_BINARY_DIR}/test-content.txt" "${test_file_content}")

add_test (NAME limes.files.cli.cat COMMAND limes::lfile cat test-content.txt
          WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set_tests_properties (limes.files.cli.cat PROPERTIES PASS_REGULAR_EXPRESSION "${test_file_content}")
