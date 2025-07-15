include_directories(../include)
add_library(MemoryMapFile SHARED 
	"../include/MemoryMapFile.h"
	"MemoryMapFile.cpp"   )

set_target_properties(MemoryMapFile PROPERTIES COMPILE_DEFINITIONS MEMFILE_EXPORTS)
