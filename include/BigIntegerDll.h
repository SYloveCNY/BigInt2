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

#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <compare>

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

    int getLastDigit() const;
    bool isPrime() const;
    friend BIGINTEGER_DLL_API std::ostream& operator<<(std::ostream& os, const BigInteger& num);

    

private:
    bool isZero() const;
    void removeLeadingZeros();
    auto compare_digits(const BigInteger& other) const;
    BigInteger inner_add(const BigInteger& other) const;
    BigInteger inner_sub(const BigInteger& other) const;
    BigInteger inner_mul(const BigInteger& other) const;
    std::pair<BigInteger, BigInteger> inner_div(const BigInteger& divisor) const;

private:
    std::vector<int64_t> digits;
    bool isNegative;
    static const int64_t BASE = 100000000LL;
    static const int DIGIT_WIDTH;
};
 