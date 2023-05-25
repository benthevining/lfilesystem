include (FetchContent)

FetchContent_Declare (
	Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG origin/devel
	GIT_SHALLOW ON
	FIND_PACKAGE_ARGS 3 GLOBAL)

FetchContent_MakeAvailable (Catch2)

if (catch2_SOURCE_DIR)
	list (APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
endif ()

include (Catch)

add_executable (lfilesystem_tests)

set_target_properties (lfilesystem_tests PROPERTIES MACOSX_BUNDLE OFF)

target_sources (lfilesystem_tests
	PRIVATE 
		CFile.cpp
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

target_compile_definitions (lfilesystem_tests PRIVATE "LIMES_TEST_DATA_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data\"")

catch_discover_tests (lfilesystem_tests
					  EXTRA_ARGS 
					  	--warn NoAssertions 
					  	--order rand 
					  	--verbosity high
					  TEST_PREFIX "Limes.files.unit.")

add_subdirectory (StructuredTree)
add_subdirectory (ExecLocation)
# add_subdirectory (DynamicLibrary)
