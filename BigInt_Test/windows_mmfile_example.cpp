#include <windows.h>
#include <iostream>
#include <string>

int main() {
    // 打开文件
    HANDLE hFile = CreateFile("primes.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();
        std::cerr << "Failed to open file. Error code: " << errorCode << std::endl;
        return 1;
    }

    // 获取文件大小
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize)) {
        std::cerr << "无法获取文件大小: " << GetLastError() << std::endl;
        CloseHandle(hFile);
        return 1;
    }
            
    // 创建文件映射对象
    HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if (hMapFile == NULL) {
        DWORD errorCode = GetLastError();
        std::cerr << "Failed to create file mapping object. Error code: " << errorCode << std::endl;
        CloseHandle(hFile);
        return 1;
    }

	// 映射视图到进程的地址空间
    int* s_primes = static_cast<int*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0));
    if (s_primes == NULL) {
        DWORD errorCode = GetLastError();
        std::cerr << "Failed to map view of file. Error code: " << errorCode << std::endl;
        CloseHandle(hMapFile);
        CloseHandle(hFile);
        return 1;
    }

    // 假设素数以 int 类型存储，计算素数数量
    size_t primeCount = fileSize.QuadPart / sizeof(int);

    // 像使用数组一样访问素数
    for (size_t i = 0; i < primeCount; ++i) {
        std::cout << "Prime " << i << ": " << s_primes[i] << std::endl;
    }

    // 访问映射区域
    //std::string data(static_cast<char*>(pBuf));
    //std::cout << "File content: " << data << std::endl;

    // 解除映射
    UnmapViewOfFile(s_primes);
    // 关闭文件映射对象
    CloseHandle(hMapFile);
    // 关闭文件句柄
    CloseHandle(hFile);

    return 0;
}    