#include <iomanip>
#include "BigIntegerDll.h"

const int BigInteger::DIGIT_WIDTH = 9;
const int64_t BigInteger::BASE = 10'0000'0000LL;

BigInteger::BigInteger(int64_t num ) : isNegative(num < 0) {
    num = std::abs(num);
    do {
        digits.push_back(num % BASE);
        num /= BASE;
    } while (num > 0);
    removeLeadingZeros();
}

bool BigInteger::isZero() const {
    return digits.size() == 1 && digits[0] == 0;
}

void BigInteger::removeLeadingZeros() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
}

auto BigInteger::compare_digits(const BigInteger& other) const
{
    if (digits.size() < other.digits.size())
        return std::strong_ordering::less;
    else if (digits.size() > other.digits.size())
        return std::strong_ordering::greater;

    for (int i = digits.size() - 1; i >= 0; --i)
    {
        if (digits[i] < other.digits[i])
            return std::strong_ordering::less;
        else if (digits[i] > other.digits[i])
            return std::strong_ordering::greater;
    }

    return std::strong_ordering::equal;
}

BigInteger BigInteger::inner_add(const BigInteger& other) const {
    BigInteger result;
    result.digits.clear();
    int64_t carry = 0;
    int maxSize = std::max(digits.size(), other.digits.size());
    for (int i = 0; i < maxSize || carry; ++i) {
        if (i < digits.size()) carry += digits[i];
        if (i < other.digits.size()) carry += other.digits[i];
        result.digits.push_back(carry % BASE);
        carry /= BASE;
    }
    result.removeLeadingZeros();
    return result;
}

BigInteger BigInteger::inner_sub(const BigInteger& other) const {
    if (compare_digits(other) == std::strong_ordering::less) {
        return other.inner_sub(*this);
    }
    BigInteger result;
    result.digits.clear();
    int64_t borrow = 0;
    for (int i = 0; i < static_cast<int>(digits.size()); ++i) {
        int64_t digit1 = digits[i];
        int64_t digit2 = (i < static_cast<int>(other.digits.size())) ? other.digits[i] : 0;
        int64_t diff = digit1 - digit2 - borrow;
        if (diff < 0) {
            diff += BASE;
            borrow = 1;
        }
        else {
            borrow = 0;
        }
        result.digits.push_back(diff);
    }
    result.removeLeadingZeros();
    return result;

}

BigInteger BigInteger::inner_mul(const BigInteger& other) const {
    if (other.digits.size() > 1) {
        bool isPowerOfTen = true;
        for (size_t i = 0; i < other.digits.size() - 1; ++i) {
            if (other.digits[i] != 0) {
                isPowerOfTen = false;
                break;
            }
        }
        if (isPowerOfTen && other.digits.back() == 1) {
            BigInteger result = *this;
            for (size_t i = 0; i < other.digits.size() - 1; ++i) {
                result.digits.insert(result.digits.begin(), 0);
            }
            result.removeLeadingZeros();
            return result;
        }
    }

    BigInteger result;
    result.digits.clear();
    result.digits.resize(digits.size() + other.digits.size());
    

    for (size_t i = 0; i < digits.size(); ++i) {
        int64_t carry = 0;
        for (size_t j = 0; j < other.digits.size() || carry; ++j) {
            int64_t cur = result.digits[i + j] +
                static_cast<int64_t>(digits[i]) * (j < other.digits.size() ? other.digits[j] : 0) + carry;
            result.digits[i + j] = static_cast<int64_t>(cur % BASE);
            carry = static_cast<int64_t>(cur / BASE);
        }
    }
    result.removeLeadingZeros();
    return result;
}

std::pair<BigInteger, BigInteger> BigInteger::inner_div(const BigInteger& divisor) const {
    if (divisor.digits.size() > 1) {
        bool isPowerOfTen = true;
        for (size_t i = 0; i < divisor.digits.size() - 1; ++i) {
            if (divisor.digits[i] != 0) {
                isPowerOfTen = false;
                break;
            }
        }
        if (isPowerOfTen && divisor.digits.back() == 1) {
            BigInteger quotient;
            quotient.digits.clear();
            BigInteger remainder;
            remainder.digits.clear();
            size_t shift = divisor.digits.size() - 1;
            if (shift >= digits.size()) {
                quotient = BigInteger(0);
                remainder = *this;
            }
            else {
                for (size_t i = shift; i < digits.size(); ++i) {
                    quotient.digits.push_back(digits[i]);
                }
                quotient.removeLeadingZeros();
                for (size_t i = 0; i < shift; ++i) {
                    remainder.digits.push_back(digits[i]);
                }
                remainder.removeLeadingZeros();
            }
            return { quotient, remainder };
        }
    }

    BigInteger quotient;
    BigInteger remainder;
    remainder.digits.clear();

    for (int i = digits.size() - 1; i >= 0; --i) {
        remainder.digits.insert(remainder.digits.begin(), digits[i]);
        remainder.removeLeadingZeros();
        int64_t left = 0, right = BASE - 1;
        int64_t q = 0;
        while (left <= right) {
            int64_t mid = (left + right) / 2;
            BigInteger temp = divisor * mid;
            if (temp.compare_digits(remainder) != std::strong_ordering::greater) {
                q = mid;
                left = mid + 1;
            }
            else {
                right = mid - 1;
            }
        }
        quotient.digits.insert(quotient.digits.begin(), q);
        remainder = remainder - divisor * q;
    }
    quotient.removeLeadingZeros();
    return { quotient, remainder };
}

auto BigInteger::operator<=>(const BigInteger& other) const
{
    if (isNegative != other.isNegative)
    {
        return isNegative ? std::strong_ordering::less : std::strong_ordering::greater;
    }

    auto ret = compare_digits(other);
    if (isNegative && ret != std::strong_ordering::equal)
    {
        ret = (ret == std::strong_ordering::less)
            ? std::strong_ordering::greater
            : std::strong_ordering::less;
    }

    return ret;
}

bool BigInteger::operator==(const BigInteger& other) const {
    return digits == other.digits;
}

bool BigInteger::operator==(int64_t other) const {
    bool isNegative2 = other < 0;
    if (isNegative != isNegative2)
        return false;

    int i = 0;
    int64_t other2 = isNegative2 ? -other : other;
    std::lldiv_t tmp = std::lldiv(other2, BASE);
    while (tmp.quot > 0)
    {
        if (digits[i] != tmp.rem)
            return false;

        ++i;
        tmp = std::lldiv(tmp.quot, BASE);
    }

    return i == digits.size() - 1 && (digits[i] == tmp.rem);
	return true;
}

BigInteger BigInteger::operator+(const BigInteger& other) const {
    if (isNegative == other.isNegative) {
        BigInteger result = inner_add(other);
        result.isNegative = isNegative;
        return result;
    }
    else {
        if (isNegative) {
            BigInteger temp = *this;
            temp.isNegative = false;
            return other - temp;
        }
        else {
            BigInteger temp = other;
            temp.isNegative = false;
            return *this - temp;
        }
    }
}

BigInteger BigInteger::operator-(const BigInteger& other) const {
    if (isNegative == other.isNegative) {
        if (compare_digits(other) == std::strong_ordering::less) {
            BigInteger result = other.inner_sub(*this);
            result.isNegative = !isNegative;
            return result;
        }
        BigInteger result = inner_sub(other);
        result.isNegative = isNegative;
        return result;
    }
    else {
        if (isNegative) {
            BigInteger temp = *this;
            temp.isNegative = false;
            BigInteger result = temp + other;
            result.isNegative = true;
            return result;
        }
        else {
            BigInteger temp = other;
            temp.isNegative = false;
            return *this + temp;
        }
    }
}

BigInteger BigInteger::operator*(const BigInteger& other) const {
    BigInteger result = inner_mul(other);
    result.isNegative = isNegative != other.isNegative;
    return result;
}

BigInteger BigInteger::operator/(const BigInteger& other) const {
    if (other.isZero()) {
        throw std::invalid_argument("Division by zero");
    }
    BigInteger absDividend = *this;
    absDividend.isNegative = false;
    BigInteger absDivisor = other;
    absDivisor.isNegative = false;
    auto [quotient, _] = absDividend.inner_div(absDivisor);
    quotient.isNegative = isNegative != other.isNegative;
    return quotient;
}

BigInteger BigInteger::operator%(const BigInteger& other) const {
    if (other.isZero()) {
        throw std::invalid_argument("Modulo by zero");
    }
    BigInteger absDividend = *this;
    absDividend.isNegative = false;
    BigInteger absDivisor = other;
    absDivisor.isNegative = false;
    auto [_, remainder] = absDividend.inner_div(absDivisor);
    remainder.isNegative = isNegative;
    return remainder;
}

int32_t BigInteger::getLastDigit() const {
    return digits[0];
}

bool BigInteger::isPrime() const { 
    BigInteger div;
    return isPrime(div);
}

bool BigInteger::isPrime(BigInteger& divisor2) const {
    if (*this == 2 || *this == 3 || *this == 5) {
        return true;
    }
    int64_t lastDigit = getLastDigit();
    if ((lastDigit % 2) == 0 || (lastDigit % 5) == 0) {
        return false;
    }

    BigInteger limit = *this;
    BigInteger divisor(3);
    while (divisor * divisor <= limit) {
        if (*this % divisor == BigInteger(0)) {
            divisor2 = divisor;
            return false;
        }
        divisor = divisor + BigInteger(2);
    }
    return true;
}

BIGINTEGER_DLL_API std::ostream& operator<<(std::ostream& os, const BigInteger& num) {
    if (num.isNegative && !(num.digits.size() == 1 && num.digits[0] == 0)) {
        os << '-';
    }
	auto it = num.digits.rbegin();
	os << *it;
	for (++it; it != num.digits.rend(); ++it) {
		os << std::setw(BigInteger::DIGIT_WIDTH) << std::setfill('0') << *it;
        }
    return os;
}


