 #include "BigInteger.h"

const int64_t BigInteger::BASE = 10'0000'0000LL;
const int BigInteger::DIGIT_WIDTH = 9;
const int BigInteger::PRECOMPUTED_PRIME_COUNT = 3;
BigInteger BigInteger::precomputedPrimes[PRECOMPUTED_PRIME_COUNT] = {
       BigInteger(3),
       BigInteger(7),
       BigInteger(11)
};
uint32_t* BigInteger::s_primes = nullptr;
size_t BigInteger::prime_count = 0;
MemoryMapFile BigInteger::s_mem_file;

BigInteger::BigInteger(uint64_t num ) : isNegative(num < 0) {
    do {
        digits.push_back(num % BASE);
        num /= BASE;
    } while (num > 0);
    removeLeadingZeros();
}

int BigInteger::compareAbs(const BigInteger& other) const {
    if (digits.size() != other.digits.size()) {
        return static_cast<int>(digits.size()) - static_cast<int>(other.digits.size());
    }
    for (int i = digits.size() - 1; i >= 0; --i) {
        if (digits[i] != other.digits[i]) {
            return digits[i] - other.digits[i];
        }
    }
    return 0;
}

bool BigInteger::isZero() const {
    return digits.size() == 1 && digits[0] == 0;
}

void BigInteger::removeLeadingZeros() {
    while (digits.size() > 1 && digits.back() == 0) {
        digits.pop_back();
    }
}

int32_t BigInteger::getLastDigit() const {
    return digits[0];
}

int32_t BigInteger::digitSum() const {
    int64_t sum = 0;
    for (int32_t digit : digits) {
        int32_t temp = digit;
        while (temp > BASE) {
            sum += temp % BASE;
            temp /= BASE;
        }
    }
    return sum;
}

bool BigInteger::isLastDigitDivisibleBy2Or5() const {
    if (digitCount == 0) return false;
    int32_t lastDigit = digits[0] % 10;
    return lastDigit % 2 == 0 || lastDigit % 5 == 0;
}

bool BigInteger::isSpecialCase(BigInteger& divisor) const {
    if (*this < 2) {
        return false;
    }
    if (*this == 2 || *this == 3 || *this == 5) {
        return true;
    }
    if (isLastDigitDivisibleBy2Or5()) {
        divisor = 2;
        if (*this % 5 == 0) {
            divisor = 5;
        }
        return false;
    }
    if (digitSum() % 3 == 0) {
        divisor = 3;
        return false;
    }
    return false;
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
    result.digits.reserve(std::max(digits.size(), other.digits.size()) + 1);
    int64_t carry = 0;
    size_t commonSize = std::min(digits.size(), other.digits.size());

    // 处理共同的位
    for (size_t i = 0; i < commonSize; ++i) {
        carry += digits[i] + other.digits[i];
        result.digits.push_back(carry % BASE);
        carry /= BASE;
    }

    // 处理第一个大整数剩余的位
    if (digits.size() > commonSize) {
        for (size_t i = commonSize; i < digits.size(); ++i) {
            carry += digits[i];
            result.digits.push_back(carry % BASE);
            carry /= BASE;
        }
    }
    // 处理第二个大整数剩余的位
    else if (other.digits.size() > commonSize) {
        for (size_t i = commonSize; i < other.digits.size(); ++i) {
            carry += other.digits[i];
            result.digits.push_back(carry % BASE);
            carry /= BASE;
        }
    }

    // 处理最后可能的进位
    if (carry > 0) {
        result.digits.push_back(carry);
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
            BigInteger remainder;
            size_t shift = divisor.digits.size() - 1;
            if (shift >= digits.size()) {
                quotient.digits = {0};
                remainder.digits = digits;
            }
            else {
                quotient.digits.assign(digits.begin() + shift, digits.end());
                remainder.digits.assign(digits.begin(), digits.begin() + shift);
            }
                quotient.removeLeadingZeros();
                remainder.removeLeadingZeros();
                return { quotient, remainder };
        }
    }

    BigInteger quotient;
    BigInteger remainder;
    remainder.digits.reserve(digits.size());

    for (int i = digits.size() - 1; i >= 0; --i) {
        remainder.digits.insert(remainder.digits.begin(), digits[i]);
        remainder.removeLeadingZeros();

		if (remainder.compare_digits(divisor) == std::strong_ordering::less) {
			quotient.digits.insert(quotient.digits.begin(), 0);
			continue;
		}

        int64_t left = 0, right = BASE - 1;
        int64_t q = 0;
        while (left <= right) {
            int64_t mid = left + ((right - left) >> 1); // 使用位运算代替除法
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

bool BigInteger::operator<(const BigInteger& other) const {
    return (*this <=> other) == std::strong_ordering::less;
}

bool BigInteger::operator<(int64_t other) const {
    if (isNegative) {
        if (other >= 0) {
            return true;  // 负数小于非负数
        }
        // 两个负数比较绝对值
        BigInteger absOther(-other);
        return compareAbs(absOther) > 0;
    }
    else {
        if (other < 0) {
            return false;  // 正数大于负数
        }
        // 两个正数比较
        BigInteger absOther(other);
        return compareAbs(absOther) < 0;
    }
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

bool BigInteger::isPrime() const { 
    BigInteger divisor;
    return isPrime(divisor);
}

bool BigInteger::isPrime(int32_t* primes, int primeCount, BigInteger& divisor) const {

    //如果大于等于 2，则返回 true，表示该数是素数；如果小于 2，则返回 false，表示该数不是素数
    if (isSpecialCase(divisor)) {
        return *this >= 2;
    }

 
    BigInteger limit = *this;
    for (int i = 0; i < primeCount; ++i) {
        BigInteger prime(precomputedPrimes[i]);
        BigInteger primeSquared = prime * prime;
        if (primeSquared > limit) {
            break;
        }
        if (*this % prime == BigInteger(0)) {
            return false;
        }
    }
    return true;
}

bool BigInteger::isPrime(BigInteger& divisor) const noexcept {
    //特殊情况
    if (isSpecialCase(divisor)) {
        return *this >= 2;
    }
    //尝试加载素数文件
    if (!s_primes) { // 仅在首次调用时加载
        size_t fileSize = 0;
        s_primes = static_cast<uint32_t*>(s_mem_file.loadFile(L"primes.dat", fileSize));
        if (s_primes) {
            prime_count = fileSize / sizeof(uint32_t);
        }
    }
    // 使用素数文件进行快速判断
    if (s_primes) {
        if (*this < std::numeric_limits<uint32_t>::max()
            && std::ranges::binary_search(s_primes, s_primes + prime_count, *this)) {
            return true;
        }

        for (size_t i = 0; i < prime_count; ++i) {
            uint64_t x = s_primes[i];
            if (x * x > *this) {
                break; 
            }
            if ((*this % x) == 0) {
                divisor = x;
                return false;
            }
        }

        uint32_t last_prime = s_primes[prime_count - 1];

        static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
        int step_count = sizeof(step) / sizeof(step[0]);

        int v = (last_prime - 7) % 30;
        int step_index = 0;
        while (v > 0) {
            v -= step[step_index];
            ++step_index;
            step_index %= step_count;
        }

        BigInteger x = last_prime;
        while ((*this / x) > x) {
            if ((*this % x) == 0) {
                divisor = x;
                return false;
            }
            x = BigInteger(step[step_index]) + x;
            ++step_index;
            step_index %= step_count;
        }
        return true;
    }

    // 素数文件加载失败，从 7 开始判断
    static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
    int step_count = sizeof(step) / sizeof(step[0]);
    BigInteger x = 7;
    int step_index = 0;
    while (x * x <= *this) {
        if ((*this % x) == 0) {
            divisor = x;
            return false;
        }
        x = BigInteger(step[step_index]) + x;
        ++step_index;
        step_index %= step_count;
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
