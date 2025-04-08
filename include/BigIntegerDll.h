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


class BIGINTEGER_DLL_API BigInteger
{
public:
    BigInteger(int64_t num = 0);
    auto operator<=>(const BigInteger& other) const;
    bool operator==(const BigInteger& other) const;
    bool operator==(int64_t other) const;
    bool operator<(const BigInteger& other) const;
    bool operator<(int64_t other) const;
    BigInteger operator+(const BigInteger& other) const;
    BigInteger operator-(const BigInteger& other) const;
    BigInteger operator*(const BigInteger& other) const;
    BigInteger operator/(const BigInteger& other) const;
    BigInteger operator%(const BigInteger& other) const;  
	bool isPrime(int32_t* primes, int primeCount) const;
    //bool isPrime(int32_t num, int primeCount) const;
    bool isPrime() const;
    bool isPrime(BigInteger& divisor2, std::vector<BigInteger>& primes) const;
    bool is_prime(uint64_t value, uint64_t& divisor) noexcept;
	void generatePrimesToFile(const char* filename, int32_t maxPrime);
	bool readPrimesFromFile();
    friend BIGINTEGER_DLL_API std::ostream& operator<<(std::ostream& os, const BigInteger& num);

    

private:
    bool isZero() const;
    void removeLeadingZeros();
    int32_t getLastDigit() const;
    int32_t digitSum() const;
    bool isLastDigitDivisibleBy2Or5() const;
    bool isSpecialCase() const;
    auto compare_digits(const BigInteger& other) const;
    BigInteger inner_add(const BigInteger& other) const;
    BigInteger inner_sub(const BigInteger& other) const;
    BigInteger inner_mul(const BigInteger& other) const;
    std::pair<BigInteger, BigInteger> inner_div(const BigInteger& divisor) const;
    

private:
    std::vector<int32_t> digits;
    bool isNegative;
    static const int64_t BASE;
    static const int DIGIT_WIDTH;
	static const int PRECOMPUTED_PRIME_COUNT;
	static BigInteger precomputedPrimes[];
    int digitCount;
    // 全局变量用于存储从文件中读取的素数
    uint32_t* s_primes;
    size_t prime_count0;
    // 全局变量用于存储文件映射对象句柄
    HANDLE hMapFile;
};
 