#define NOMINMAX
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <ranges>
#include <algorithm>
#include <limits>

// 全局变量用于存储从文件中读取的素数
uint32_t* sPrimes = nullptr;
size_t primeCount = 0;
// 全局变量用于存储文件映射对象句柄
HANDLE hMapFile = NULL;

// 读取素数文件
static bool loadPrimesFromFile() {
	// 打开文件
	HANDLE hFile = CreateFile("primes.dat", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
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
	HANDLE hMappedFile = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (hMappedFile == NULL) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to create file mapping object. Error code: " << errorCode << std::endl;
		CloseHandle(hFile);
		return false;
	}

	// 映射视图到进程的地址空间
	sPrimes = static_cast<uint32_t*>(MapViewOfFile(hMappedFile, FILE_MAP_ALL_ACCESS, 0, 0, 0));
	if (sPrimes == NULL) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to map view of file. Error code: " << errorCode << std::endl;
		CloseHandle(hMappedFile);
		CloseHandle(hFile);
		return false;
	}

	// 假设素数以 int 类型存储，计算素数数量
	primeCount = fileSize.QuadPart / sizeof(uint32_t);

	// 关闭文件句柄
	CloseHandle(hFile);

	return true;
}

static bool isPrime(uint64_t value, uint64_t& divisor) noexcept
{
	if (value == 2 || value == 3 || value == 5)
		return true;

	for (auto x : { 2, 3, 5 })
	{
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}
	}

	if (value < std::numeric_limits<int32_t>::max()
		&& std::ranges::binary_search(sPrimes, sPrimes + primeCount, value))
		return true;

	for (size_t i = 0; i < primeCount; ++i)
	{
		uint64_t x = sPrimes[i];
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}

		if (value / x < x)
			return true;
	}

	uint32_t lastPrime = sPrimes[primeCount - 1];

	static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
	int stepCount = sizeof(step) / sizeof(step[0]);

	int v = (lastPrime - 7) % 30;
	int stepIndex = 0;
	while (v > 0)
	{
		v -= step[stepIndex];
		++stepIndex;
		stepIndex %= stepCount;
	}

	uint64_t x = lastPrime;
	while ((value / x) > x)
	{
		if ((value % x) == 0)
		{
			divisor = x;
			return false;
		}

		x += step[stepIndex];
		++stepIndex;
		stepIndex %= stepCount;
	}

	return true;
}

int main(int argc, char* argv[])
{


	if (!loadPrimesFromFile()) {
		return 1;
	}
	std::cout << sPrimes[0] << ',' << sPrimes[1] << ',' << sPrimes[2] << std::endl;
	uint64_t value = std::numeric_limits<uint64_t>::max();
	value -= 14;

	uint64_t divisor = 0;
	bool ret = isPrime(value, divisor);
	if (ret)
		std::cout << value << ", 是素数。" << std::endl;
	else
		std::cout << value << ", 可以被 " << divisor << " 整除。" << std::endl;

	// 解除映射
	UnmapViewOfFile(sPrimes);
	// 关闭文件映射对象
	CloseHandle(hMapFile);

	return 0;
};

