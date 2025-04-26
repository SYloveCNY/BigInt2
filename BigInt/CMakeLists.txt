include_directories(../include)
add_library(BigInt SHARED
	../include/BigInteger.h
	BigInteger.cpp  )

set_target_properties(BigInt PROPERTIES COMPILE_DEFINITIONS BIGINTEGER_DLL_EXPORTS)
target_link_libraries(BigInt MemoryMapFile)

