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

add_test (NAME limes.files.cli.basename COMMAND limes::lfile basename
												"${CMAKE_CURRENT_LIST_DIR}/foo")

set_tests_properties (limes.files.cli.basename PROPERTIES PASS_REGULAR_EXPRESSION
														  "${CMAKE_CURRENT_LIST_DIR}")
