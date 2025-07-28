#include "MemoryMapFile.h"

void* MemoryMapFile::loadFile(const FileNameType& fileName, size_t& fileSize) {
	#ifdef _WIN32
	// 打开文件
	_file_handle = CreateFileW(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file_handle == INVALID_HANDLE_VALUE) {
		std::cerr << "Failed to open file on Windows." << std::endl;
		return nullptr;
	}

	// 获取文件大小
	LARGE_INTEGER getFileSize;
	if (!GetFileSizeEx(_file_handle, &getFileSize)) {
		std::cerr << "Failed to get file size on Windows." << std::endl;
		CloseHandle(_file_handle);
		return nullptr;
	}
	fileSize = static_cast<size_t>(getFileSize.QuadPart);

	// 创建文件映射对象
	_map_handle = CreateFileMappingW(_file_handle, NULL, PAGE_READONLY, 0, 0, NULL);
	if (_map_handle == NULL) {
		std::cerr << "Failed to create file mapping on Windows." << std::endl;
		CloseHandle(_file_handle);
		return nullptr;
	}

	// 映射视图
	void* mappedAddress = MapViewOfFile(_map_handle, FILE_MAP_READ, 0, 0, 0);
	if (mappedAddress == NULL) {
		std::cerr << "Failed to map view of file on Windows." << std::endl;
		CloseHandle(_map_handle);
		CloseHandle(_file_handle);
		return nullptr;
	}

	return mappedAddress;
	#else
	// 打开文件
	_file_handle = open(fileName.c_str(), O_RDONLY);
	if (_file_handle == -1) {
		std::cerr << "Failed to open file on non-Windows." << std::endl;
		return nullptr;
	}

	// 获取文件大小
	fileSize = lseek(_file_handle, 0, SEEK_END);
	lseek(_file_handle, 0, SEEK_SET);

	// 内存映射
	void* mappedAddress = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, _file_handle, 0);
	if (mappedAddress == MAP_FAILED) {
		std::cerr << "Failed to map file on non-Windows." << std::endl;
		close(_file_handle);
		return nullptr;
	}

	return mappedAddress;
	#endif
}

void MemoryMapFile::unLoad() {
	#ifdef _WIN32
	if (_map_handle != NULL) {
		UnmapViewOfFile(_map_handle);
		CloseHandle(_map_handle);
	}
	if (_file_handle != INVALID_HANDLE_VALUE) {
		CloseHandle(_file_handle);
	}
	#else
	if (_file_handle != -1) {
		if (munmap(reinterpret_cast<void*>(_map_handle), _file_handle) == -1) {
			std::cerr << "Failed to unmap file on non-Windows." << std::endl;
		}
		close(_file_handle);
	}
	#endif
}