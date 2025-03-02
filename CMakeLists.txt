cmake_minimum_required(VERSION 3.28)

project(BigInt
		LANGUAGES CXX
		VERSION 1.0.0.0
		DESCRIPTION "效率和内存占用都优化到极致的大整数。")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(BigInt)
add_subdirectory(BigInt_Test)
