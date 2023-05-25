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

add_library (limes_test_shared_lib SHARED)

target_sources (limes_test_shared_lib PRIVATE shared_lib.cpp)

target_link_libraries (limes_test_shared_lib PUBLIC Limes::limes_files)

target_compile_definitions (
	limes_files_tests PRIVATE "LIMES_TEST_DYLIB_PATH=\"$<TARGET_FILE:limes_test_shared_lib>\"")
