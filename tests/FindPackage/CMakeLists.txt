cmake_minimum_required (VERSION 3.25 FATAL_ERROR)

project (lfilesystem_findpackage_test
		 VERSION 0.0.1
		 LANGUAGES CXX)

find_package (lfilesystem REQUIRED)

add_executable (test)

target_sources (test PRIVATE main.cpp)

target_link_libraries (test PRIVATE limes::lfilesystem)
