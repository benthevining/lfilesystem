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

function (lfilesystem_copy_dlls target)
	if (WIN32)
		add_custom_command (
			TARGET "${target}" POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy -t $<TARGET_FILE_DIR:${target}>
					$<TARGET_RUNTIME_DLLS:${target}> USES_TERMINAL COMMAND_EXPAND_LISTS)
	endif ()
endfunction ()

#

set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")
set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO")
set (CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "")

include (FetchContent)

if ("${CMAKE_SYSTEM_NAME}" MATCHES "tvOS" OR "${CMAKE_SYSTEM_NAME}" MATCHES "watchOS")
	set (CATCH_CONFIG_NO_POSIX_SIGNALS ON)
endif ()

FetchContent_Declare (
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG origin/devel
	GIT_SHALLOW ON
	FIND_PACKAGE_ARGS 3 GLOBAL)

FetchContent_MakeAvailable (Catch2)

if (TARGET Catch2 AND "${CMAKE_SYSTEM_NAME}" MATCHES "watchOS")
	target_compile_options (Catch2 PRIVATE -fgnu-inline-asm)
endif ()

if (catch2_SOURCE_DIR)
	list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif ()

include (Catch)

add_executable (lfilesystem_tests)

if (IOS)
	set (generated_plist "${CMAKE_CURRENT_BINARY_DIR}/Info.plist")

	configure_file (Info.plist "${generated_plist}" @ONLY NEWLINE_STYLE UNIX)

	set_target_properties (
		lfilesystem_tests
		PROPERTIES BUNDLE TRUE
				   MACOSX_BUNDLE TRUE
				   XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
				   XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
				   XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
				   XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "Limes.lfilesystem.tests"
				   MACOSX_FRAMEWORK_BUNDLE_VERSION "${lfilesystem_VERSION}"
				   MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${lfilesystem_VERSION_MAJOR}"
				   MACOSX_BUNDLE_INFO_PLIST "${generated_plist}")
else ()
	set_target_properties (lfilesystem_tests PROPERTIES MACOSX_BUNDLE OFF)
endif ()

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

target_link_libraries (lfilesystem_tests PRIVATE Catch2::Catch2WithMain limes::lfilesystem)

target_compile_definitions (lfilesystem_tests
							PRIVATE "LIMES_TEST_DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data\"")

lfilesystem_copy_dlls (lfilesystem_tests)

# cmake-format: off
catch_discover_tests (
	lfilesystem_tests
	EXTRA_ARGS
		--warn NoAssertions
		--order rand
		--verbosity high
	TEST_PREFIX "limes.files.unit.")
# cmake-format: on

if (NOT (EMSCRIPTEN OR IOS))
	add_subdirectory (StructuredTree)
	add_subdirectory (ExecLocation)
	# add_subdirectory (DynamicLibrary)
endif ()

# include (FindPackage/CreateTests.cmake)
