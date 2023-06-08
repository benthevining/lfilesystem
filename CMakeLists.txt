cmake_minimum_required (VERSION 3.25 FATAL_ERROR)

project (
	lfilesystem
	VERSION 0.0.1
	LANGUAGES CXX C
	DESCRIPTION "C++ filesystem library"
	HOMEPAGE_URL "https://github.com/benthevining/lfilesystem/")

add_subdirectory (linfra)

limes_default_project_options ()

option (LFILE_TESTS "Build the lfilesystem tests" "${lfilesystem_IS_TOP_LEVEL}")

option (LFILE_DOCS "Build the lfilesystem docs" "${lfilesystem_IS_TOP_LEVEL}")

include (CMakeDependentOption)

cmake_dependent_option (LFILE_CLI "Build the lfilesystem CLI app" "${lfilesystem_IS_TOP_LEVEL}"
						"NOT IOS" OFF)

include (GNUInstallDirs)

set (LFILE_INSTALL_DEST "${CMAKE_INSTALL_LIBDIR}/cmake/lfilesystem"
	 CACHE STRING "Path where package files will be installed, relative to the install prefix")

mark_as_advanced (LFILE_INSTALL_DEST LFILE_TESTS LFILE_DOCS LFILE_CLI)

add_library (lfilesystem)
add_library (limes::lfilesystem ALIAS lfilesystem)

target_compile_features (lfilesystem PUBLIC cxx_std_20)

set_target_properties (lfilesystem PROPERTIES DEBUG_POSTFIX -d VERSION "${lfilesystem_VERSION}"
											  SOVERSION "${lfilesystem_VERSION_MAJOR}")

set (public_header include/lfilesystem/lfilesystem.h)

set (
	private_headers
	include/lfilesystem/lfilesystem_CFile.h
	include/lfilesystem/lfilesystem_Directory.h
	include/lfilesystem/lfilesystem_DynamicLibrary.h
	include/lfilesystem/lfilesystem_File.h
	include/lfilesystem/lfilesystem_FilesystemEntry.h
	include/lfilesystem/lfilesystem_FileWatcher.h
	include/lfilesystem/lfilesystem_Misc.h
	include/lfilesystem/lfilesystem_Paths.h
	include/lfilesystem/lfilesystem_Permissions.h
	include/lfilesystem/lfilesystem_SimpleWatcher.h
	include/lfilesystem/lfilesystem_SpecialDirectories.h
	include/lfilesystem/lfilesystem_SymLink.h
	include/lfilesystem/lfilesystem_Volume.h)

set (generated_headers_dir "${CMAKE_CURRENT_BINARY_DIR}/generated/lfilesystem")

set (export_header "${generated_headers_dir}/lfilesystem_Export.h")

include (GenerateExportHeader)

# cmake-format: off
generate_export_header (
	lfilesystem
	EXPORT_MACRO_NAME LFILE_EXPORT
	NO_EXPORT_MACRO_NAME LFILE_NO_EXPORT
	EXPORT_FILE_NAME "${export_header}")
# cmake-format: on

set (version_header "${generated_headers_dir}/lfilesystem_Version.h")

configure_file ("${CMAKE_CURRENT_LIST_DIR}/cmake/version.in" "${version_header}" @ONLY
				NEWLINE_STYLE UNIX)

target_sources (lfilesystem # PUBLIC "${public_header}"
				PRIVATE ${private_headers} "${export_header}" "${version_header}")

target_include_directories (
	lfilesystem
	PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
		   $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated>
		   $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

install (FILES "${public_header}" "${export_header}" "${version_header}" ${private_headers}
		 DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/lfilesystem" COMPONENT lfilesystem_dev)

if (APPLE)

	enable_language (OBJCXX)

	target_link_libraries (lfilesystem PRIVATE "-framework Foundation")

	if (NOT IOS)
		target_link_libraries (lfilesystem PRIVATE "-framework AppKit")
	endif ()

elseif (WIN32)

	target_compile_definitions (lfilesystem PRIVATE NOMINMAX)

elseif (LINUX)

	include (CheckCXXSourceCompiles)

	check_cxx_source_compiles (
		[=[
#include <unistd.h>
#include <string>

int main (int, char**)
{
const std::string path { "/some/file/path.txt" };

[[maybe_unused]] auto result = pathconf (path.data(), _PC_CASE_SENSITIVE);

return 0;
}
]=]
		limes_files_can_use_pathconf)

	if (limes_files_can_use_pathconf)
		target_compile_definitions (lfilesystem PRIVATE LFILE_IMPL_USE_PATHCONF=1)
	else ()
		target_compile_definitions (lfilesystem PRIVATE LFILE_IMPL_USE_PATHCONF=0)
	endif ()

endif ()

target_link_libraries (lfilesystem PRIVATE ${CMAKE_DL_LIBS})

add_subdirectory (src)

if (LFILE_CLI)
	add_subdirectory (cli)
endif ()

install (
	TARGETS lfilesystem
	EXPORT lfilesystem
	LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lfilesystem
			NAMELINK_COMPONENT lfilesystem_dev
	ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT lfilesystem_dev
	RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT lfilesystem
	INCLUDES
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install (
	EXPORT lfilesystem
	NAMESPACE limes::
	FILE Targets.cmake
	DESTINATION "${LFILE_INSTALL_DEST}"
	COMPONENT lfilesystem_dev)

include (CMakePackageConfigHelpers)

write_basic_package_version_file (lfilesystem-config-version.cmake VERSION "${lfilesystem_VERSION}"
								  COMPATIBILITY SameMajorVersion)

configure_package_config_file (cmake/lfilesystem-config.in lfilesystem-config.cmake
							   INSTALL_DESTINATION "${LFILE_INSTALL_DEST}" NO_SET_AND_CHECK_MACRO)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/lfilesystem-config-version.cmake"
			   "${CMAKE_CURRENT_BINARY_DIR}/lfilesystem-config.cmake"
		 DESTINATION "${LFILE_INSTALL_DEST}" COMPONENT lfilesystem_dev)

include (CPackComponent)

cpack_add_component (
	lfilesystem
	DISPLAY_NAME "Limes filesystem library"
	DESCRIPTION
		"The Limes C++ filesystem library. This is the runtime component needed by end users."
	GROUP limes_files)

cpack_add_component (
	lfilesystem_dev
	DISPLAY_NAME "Limes filesystem library headers"
	DESCRIPTION
		"Development package for the Limes C++ filesystem library. This component includes the library's header files and CMake package files."
	GROUP limes_files
	DEPENDS lfilesystem)

cpack_add_component_group (limes_files DISPLAY_NAME "Limes filesystem"
						   DESCRIPTION "Limes filesystem library" PARENT_GROUP limes)

if (LFILE_TESTS)
	enable_testing ()

	add_subdirectory (tests)

	include (CTest)
endif ()

if (LFILE_DOCS)
	add_subdirectory (docs)
endif ()

if (lfilesystem_IS_TOP_LEVEL)
	include (CPack)
endif ()
