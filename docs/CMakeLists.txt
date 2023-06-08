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

limes_default_doxygen_settings ()

set (DOXYGEN_EXAMPLE_PATH "${CMAKE_CURRENT_LIST_DIR}/examples")

file (GLOB examples LIST_DIRECTORIES false CONFIGURE_DEPENDS "${DOXYGEN_EXAMPLE_PATH}/*.cpp")

if (LFILE_TESTS)
	add_library (lfilesystem_examples STATIC EXCLUDE_FROM_ALL )

	target_sources (lfilesystem_examples PRIVATE ${examples})

	target_link_libraries (lfilesystem_examples PRIVATE limes::lfilesystem)

	add_test (NAME "Limes.files.examples" COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}"
												  --config $<CONFIG> --target lfilesystem_examples)
endif ()

set (LFILE_DOC_TAGFILE "${CMAKE_CURRENT_LIST_DIR}/../doc/lfilesystem.tag"
	 CACHE FILEPATH "Path where the Doxygen tagfile for lfilesystem will be generated")

mark_as_advanced (LFILE_DOC_TAGFILE)

file (GLOB headers LIST_DIRECTORIES false CONFIGURE_DEPENDS
										  "${CMAKE_CURRENT_LIST_DIR}/../include/lfilesystem/*.h")

set (DOXYGEN_GENERATE_TAGFILE "${LFILE_DOC_TAGFILE}")

doxygen_add_docs (
	lfilesystem_docs ${headers} ${examples} "${DOXYGEN_USE_MDFILE_AS_MAINPAGE}" USE_STAMP_FILE
	WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/.."
	COMMENT "Building lfilesystem documentation...")

set_property (
	TARGET lfilesystem_docs APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${DOXYGEN_OUTPUT_DIRECTORY}"
											"${DOXYGEN_WARN_LOGFILE}" "${DOXYGEN_GENERATE_TAGFILE}")

limes_add_docs_coverage (lfilesystem_docs)
