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

add_executable (lfile)
add_executable (limes::lfile ALIAS lfile)

set (common_files main.cpp modes.cpp modes.h)

set (modes_files modes/cat.h modes/basename.h modes/cp.h modes/df.h)

target_sources (lfile PRIVATE ${common_files} ${modes_files})

source_group (common FILES ${common_files})
source_group (modes FILES ${modes_files})

target_link_libraries (lfile PRIVATE limes::lfilesystem)

target_include_directories (lfile PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

limes_copy_dlls (lfile)

install (
    TARGETS lfile
    EXPORT lfilesystem
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lfilesystem
            NAMELINK_COMPONENT lfilesystem_dev
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lfilesystem_dev
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lfilesystem
    INCLUDES
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

if (LFILE_TESTS)
    add_subdirectory (test)
endif ()
