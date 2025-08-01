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
#include <fcntl.h>    // 包含 open、O_RDONLY 等
#include <unistd.h>   // 包含 lseek、close 等
#include <sys/mman.h> // 包含 mmap、munmap、PROT_READ、MAP_PRIVATE 等
#include <sys/stat.h> // 辅助文件操作
#include <cstring>  // 包含 strerror 声明

#if defined(__unix__)
typedef std::string FileNameType;
typedef int FileHandle;
#define USTR(x) x
#else
#define NOMINMAX
#include <windows.h>
#define __WIN
typedef std::wstring FileNameType;
typedef HANDLE FileHandle;
#define USTR(x) L ## x
#endif

class MEMFILE_API MemoryMapFile {
public:

	~MemoryMapFile() { unLoad(); }

	void* loadFile(const FileNameType& fileName, size_t& fileSize);
	void unLoad();

private:
#ifdef _WIN32
	HANDLE _file_handle;
    HANDLE _map_handle;       // Windows 特有：映射句柄
#else
	int _file_handle;    // Linux文件描述符
    void* _map_address;  // Linux映射地址
    size_t _file_size;   // Linux文件大小
#endif
};

