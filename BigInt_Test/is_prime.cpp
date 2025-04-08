#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <ranges>
#include <algorithm>
#include <limits>

// 全局变量用于存储从文件中读取的素数
uint32_t* s_primes = nullptr;
size_t prime_count = 0;
// 全局变量用于存储文件映射对象句柄
HANDLE hMapFile = NULL;

// 读取素数文件
bool readPrimesFromFile() {
	// 打开文件
	HANDLE hFile = CreateFile("primes.txt", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to open file. Error code: " << errorCode << std::endl;
		return false;
	}

	// 获取文件大小
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		std::cerr << "无法获取文件大小: " << GetLastError() << std::endl;
		CloseHandle(hFile);
		return false;
	}

	// 创建文件映射对象
	HANDLE hMapFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hMapFile == NULL) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to create file mapping object. Error code: " << errorCode << std::endl;
		CloseHandle(hFile);
		return false;
	}

	// 映射视图到进程的地址空间
	s_primes = static_cast<uint32_t*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	if (s_primes == NULL) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to map view of file. Error code: " << errorCode << std::endl;
		CloseHandle(hMapFile);
		CloseHandle(hFile);
		return false;
	}

	// 假设素数以 int 类型存储，计算素数数量
	prime_count = fileSize.QuadPart / sizeof(uint32_t);

	// 关闭文件句柄
	CloseHandle(hFile);

	


	return true;
}

bool is_prime(uint64_t value, uint64_t& divisor) noexcept
{
	if (value == 2 || value == 3 || value == 5)
		return true;

	for (auto x: {2, 3, 5})
	{
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}
	}

	if (value < std::numeric_limits<int32_t>::max()
		&& std::ranges::binary_search(s_primes, s_primes + prime_count, value))
		return true;

	for (size_t i = 0; i < prime_count; ++i)
	{
		uint64_t x = s_primes[i];
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}

		if (value / x < x)
			return true;
	}

	uint32_t last_prime = s_primes[prime_count - 1];

	static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
	int step_count = sizeof(step) / sizeof(step[0]);

	int v = (last_prime - 7) % 30;
	int step_index = 0;
	while (v > 0)
	{
		v -= step[step_index];
		++ step_index;
		step_index %= step_count;
	}

	uint64_t x = last_prime;
	while ((value / x) > x)
	{
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}

		x += step[step_index];
		++ step_index;
		step_index %= step_count;
	}

	return true;
}

int main(int argc, char* argv[])
{
	if (!readPrimesFromFile()) {
		return 1;
	}

	uint64_t value = std::numeric_limits<uint64_t>::max();
	value -= 14;

	uint64_t divisor = 0;
	bool ret = is_prime(value, divisor);
	if (ret)
		std::cout << value << ", 是素数。" << std::endl;
	else
		std::cout << value << ", 可以被 " << divisor << " 整除。" << std::endl;

	// 解除映射
	UnmapViewOfFile(s_primes);
	// 关闭文件映射对象
	CloseHandle(hMapFile);

	return 0;
};

