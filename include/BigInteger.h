#pragma once
#ifdef _MSC_VER
#ifdef BIGINTEGER_DLL_EXPORTS
#define BIGINTEGER_DLL_API __declspec(dllexport)
#else
#define BIGINTEGER_DLL_API __declspec(dllimport)
#endif
#else
#define BIGINTEGER_DLL_API
#endif

#define NOMINMAX
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>
#include <filesystem>
#include <winnt.h>
#include <limits>
#include <ranges>
#include <iomanip>

#include "MemoryMapFile.h"

class BIGINTEGER_DLL_API BigInteger {
public:
	// 从64位无符号整数构造
	BigInteger(uint64_t num = 0);
	// 比较运算符
	auto operator<=>(const BigInteger& other) const;
	bool operator==(const BigInteger& other) const;
	bool operator==(int64_t other) const;
	bool operator<(const BigInteger& other) const;
	bool operator<(int64_t other) const;
	bool operator<=(const BigInteger& other) const;
	// 一元运算符
	BigInteger operator+() const;
	BigInteger operator-() const;
	// 算术运算符
	BigInteger operator+(const BigInteger& other) const;
	BigInteger operator-(const BigInteger& other) const;
	BigInteger operator*(const BigInteger& other) const;
	BigInteger operator/(const BigInteger& other) const;
	BigInteger operator%(const BigInteger& other) const;

	bool isPrimeNumber() const;
	bool isPrimeNumber(BigInteger& divisor) const noexcept;
	static BigInteger fibonacci(int64_t n);
	static BigInteger factorial(int64_t n);

	// 友元声明
	friend BIGINTEGER_DLL_API BigInteger operator"" _bi(const char* str, size_t len);
	friend BIGINTEGER_DLL_API std::ostream& operator<<(std::ostream& os, const BigInteger& num);

private:
	// 私有构造函数
	BigInteger(std::vector<int32_t>&& d, bool negative)
		: digits(std::move(d)), isNegative(negative) {
		removeLeadingZeros();
		if (isZero()) {
			isNegative = false;  // 零值强制为非负
		}
	}
	// 检查所有块是否为零
	static bool allZero(const std::vector<uint64_t>& blocks);
	int compareDigitsAbsolute(const BigInteger& other) const;
	bool isZero() const;
	void removeLeadingZeros();
	int32_t mod3() const;
	auto compareDigits(const BigInteger& other) const;
	bool checkPrimeWithStep(BigInteger& divisor, BigInteger start) const;
	BigInteger innerAdd(const BigInteger& other) const;
	BigInteger innerSub(const BigInteger& other) const;
	BigInteger innerMul(const BigInteger& other) const;
	std::pair<BigInteger, BigInteger> innerDiv(const BigInteger& divisor) const;


private:
	static const int STEP[];
	static const int STEP_COUNT;
	static const int64_t BASE;
	static const int DIGIT_WIDTH;

	std::vector<int32_t> digits;
	bool isNegative;
	int digitCount;

	static uint32_t* sPrimes;
	static size_t primeCount;
	static MemoryMapFile sMemFile;
};

class BIGINTEGER_DLL_API Matrix {
public:
	BigInteger data[2][2];

	Matrix();
	Matrix operator*(const Matrix& other) const;
	Matrix fastPower(int n) const;
};

// 全局声明添加宏
BIGINTEGER_DLL_API BigInteger operator"" _bi(const char* str, size_t len);