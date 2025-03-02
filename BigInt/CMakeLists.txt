include_directories(../include)
add_library(BigInt SHARED
	../include/BigIntegerDll.h
	BigIntegerDll.cpp "test.cpp")

set_target_properties(BigInt PROPERTIES COMPILE_DEFINITIONS BIGINTEGER_DLL_EXPORTS)
