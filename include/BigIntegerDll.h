﻿#pragma once
#ifdef _MSC_VER
#ifdef BIGINTEGER_DLL_EXPORTS
#define BIGINTEGER_DLL_API __declspec(dllexport)
#else
#define BIGINTEGER_DLL_API __declspec(dllimport)
#endif
#else
#define BIGINTEGER_DLL_API
#endif

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <ctime>

class BIGINTEGER_DLL_API BigInteger
{
public:
    BigInteger(int64_t num = 0);
    auto operator<=>(const BigInteger& other) const;
    bool operator==(const BigInteger& other) const;
    bool operator==(int64_t other) const;
    BigInteger operator+(const BigInteger& other) const;
    BigInteger operator-(const BigInteger& other) const;
    BigInteger operator*(const BigInteger& other) const;
    BigInteger operator/(const BigInteger& other) const;
    BigInteger operator%(const BigInteger& other) const;

    int32_t getLastDigit() const;
	BigInteger modPow(const BigInteger& exponent, const BigInteger& modulus) const;
    bool isPrime() const;
    bool isPrime(BigInteger& divisor2, std::vector<BigInteger>& primes) const;
	bool isPrimeMillerRabin(int k = 5) const;
    friend BIGINTEGER_DLL_API std::ostream& operator<<(std::ostream& os, const BigInteger& num);

    

private:
    bool isZero() const;
    void removeLeadingZeros();
    auto compare_digits(const BigInteger& other) const;
    BigInteger inner_add(const BigInteger& other) const;
    BigInteger inner_sub(const BigInteger& other) const;
    BigInteger inner_mul(const BigInteger& other) const;
    std::pair<BigInteger, BigInteger> inner_div(const BigInteger& divisor) const;
    bool isLastDigitDivisibleBy2Or5() const;

private:
    std::vector<int32_t> digits;
    bool isNegative;
    static const int64_t BASE;
    static const int DIGIT_WIDTH;
    static const std::vector<BigInteger> precomputedPrimes;
};
 