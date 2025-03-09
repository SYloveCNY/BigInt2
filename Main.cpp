#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <compare>

class BigInteger {
private:
    std::vector<int> digits;
    bool isNegative;

    void removeLeadingZeros() {
        while (digits.size() > 1 && digits.back() == 0) {
            digits.pop_back();
        }
    }

    auto compare_digits(const BigInteger& other) const
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

    BigInteger inner_add(const BigInteger& other) const {
        BigInteger result;
        result.digits.clear();
        int carry = 0;
        int maxSize = std::max(digits.size(), other.digits.size());
        for (int i = 0; i < maxSize || carry; ++i) {
            if (i < digits.size()) carry += digits[i];
            if (i < other.digits.size()) carry += other.digits[i];
            result.digits.push_back(carry % 10);
            carry /= 10;
        }
        result.removeLeadingZeros();
        return result;
    }

    BigInteger inner_sub(const BigInteger& other) const {
        if (compare_digits(other) == std::strong_ordering::less) {
            return other.inner_sub(*this);
        }
        BigInteger result;
        result.digits.clear();
        int borrow = 0;
        for (int i = 0; i < static_cast<int>(digits.size()); ++i) {
            int digit1 = digits[i];
            int digit2 = (i < static_cast<int>(other.digits.size())) ? other.digits[i] : 0;
            int diff = digit1 - digit2 - borrow;
            if (diff < 0) {
                diff += 10;
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

    BigInteger inner_mul(const BigInteger& other) const {
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
            int carry = 0;
            for (size_t j = 0; j < other.digits.size() || carry; ++j) {
                long long cur = result.digits[i + j] +
                    static_cast<long long>(digits[i]) * (j < other.digits.size() ? other.digits[j] : 0) + carry;
                result.digits[i + j] = static_cast<int>(cur % 10);
                carry = static_cast<int>(cur / 10);
            }
        }
        result.removeLeadingZeros();
        return result;
    }

    std::pair<BigInteger, BigInteger> inner_div(const BigInteger& divisor) const {
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
        BigInteger dividend = *this;

        for (int i = dividend.digits.size() - 1; i >= 0; --i) {
            remainder.digits.insert(remainder.digits.begin(), dividend.digits[i]);
            remainder.removeLeadingZeros();
            int q = 0;
            while (remainder.compare_digits(divisor) != std::strong_ordering::less) {
                remainder = remainder - divisor;
                ++q;
            }
            quotient.digits.insert(quotient.digits.begin(), q);
        }
        quotient.removeLeadingZeros();
        return { quotient, remainder };
    }


public:
    BigInteger(int num = 0) : isNegative(num < 0) {
        num = std::abs(num);
        do {
            digits.push_back(num % 10);
            num /= 10;
        } while (num > 0);
        removeLeadingZeros();
    }

    auto operator<=>(const BigInteger& other) const
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

    bool operator==(const BigInteger& other) const {
        return digits == other.digits;
    }

    BigInteger operator+(const BigInteger& other) const {
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

    BigInteger operator-(const BigInteger& other) const {
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

    BigInteger operator*(const BigInteger& other) const {
        BigInteger result = inner_mul(other);
        result.isNegative = isNegative != other.isNegative;
        return result;
    }

    BigInteger operator/(const BigInteger& other) const {
        if (other.digits.size() == 1 && other.digits[0] == 0) {
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

    BigInteger operator%(const BigInteger& other) const {
        if (other.digits.size() == 1 && other.digits[0] == 0) {
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

    friend std::ostream& operator<<(std::ostream& os, const BigInteger& num) {
        if (num.isNegative && !(num.digits.size() == 1 && num.digits[0] == 0)) {
            os << '-';
        }
        for (auto it = num.digits.rbegin(); it != num.digits.rend(); ++it) {
            os << *it;
        }
        return os;
    }
};

BigInteger fibonacci(int n) {
    if (n == 0) return BigInteger(0);
    if (n == 1) return BigInteger(1);
    BigInteger a(0);
    BigInteger b(1);
    BigInteger result;
    for (int i = 2; i <= n; ++i) {
        result = a + b;
        a = b;
        b = result;
    }
    return result;
}

BigInteger factorial(int n) {
    if (n < 0) {
        throw std::invalid_argument("Factorial is not defined for negative numbers.");
    }
    BigInteger result(1);
    for (int i = 2; i <= n; ++i) {
        result = result * BigInteger(i);
    }
    return result;
}


int main() {
    BigInteger num1(987654321);
    BigInteger num2(123456789);

    std::cout << "num1: " << num1 << std::endl;
    std::cout << "num2: " << num2 << std::endl;

    BigInteger sum = num1 + num2;
    std::cout << "Sum1: " << sum << std::endl;

    BigInteger diff = num1 - num2;
    std::cout << "Difference: " << diff << std::endl;

    BigInteger cheng = num1 * num2;
    std::cout << "Cheng: " << cheng << std::endl;

    BigInteger chu = num1 / num2;
    std::cout << "Chu: " << chu << std::endl;

    BigInteger quyu = num1 % num2;
    std::cout << "Quyu: " << quyu << std::endl;

    int n = 100;
    BigInteger fib = fibonacci(n);
    std::cout << "Fibonacci(" << n << ") = " << fib << std::endl;

    int x = 20;
    BigInteger fact = factorial(x);
    std::cout << x << "! = " << fact << std::endl;

    return 0;
}
