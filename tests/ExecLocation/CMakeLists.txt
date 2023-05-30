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

add_executable (limes_files_exec_location_printer)

target_sources (limes_files_exec_location_printer PRIVATE main.cpp)

target_link_libraries (limes_files_exec_location_printer PRIVATE limes::lfilesystem)

set_target_properties (limes_files_exec_location_printer PROPERTIES MACOSX_BUNDLE OFF)

add_executable (limes::limes_files_exec_location_printer ALIAS limes_files_exec_location_printer)

lfilesystem_copy_dlls (limes_files_exec_location_printer)

# Need to send the correct path as an argument to the test so that we can use generator expressions
add_test (NAME limes.files.ExecLocation COMMAND limes::limes_files_exec_location_printer
												"$<TARGET_FILE:limes_files_exec_location_printer>")
