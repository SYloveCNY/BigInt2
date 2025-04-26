#pragma once
#ifdef _MSC_VER
#ifdef MEMFILE_EXPORTS
#define MEMFILE_API __declspec(dllexport)
#else
#define MEMFILE_API __declspec(dllimport)
#endif
#else
#define MEMFILE_API
#endif

#include <iostream>
#include <fstream>
#include <cstdint>
#include <string>

#if defined(__unix__)
typedef std::string FileNameType;
typedef xxx FileHandle;
#else
#define NOMINMAX
#include <windows.h>
#define __WIN
typedef std::wstring FileNameType;
typedef HANDLE FileHandle;
#endif

class MEMFILE_API MemoryMapFile {
public:

	~MemoryMapFile() { unload(); }

	void* loadFile(const FileNameType& fileName, size_t& fileSize); 
	void unload();
	
private:
	FileHandle _map_handle;
	FileHandle _file_handle;
};

