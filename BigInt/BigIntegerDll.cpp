#include <iomanip>
#include "BigIntegerDll.h"

const int BigInteger::DIGIT_WIDTH = 9;
const int64_t BigInteger::BASE = 10'0000'0000LL;
const int BigInteger::PRECOMPUTED_PRIME_COUNT = 3;
BigInteger BigInteger::precomputedPrimes[PRECOMPUTED_PRIME_COUNT] = {
       BigInteger(3),
       BigInteger(7),
       BigInteger(11)
};
// 全局变量用于存储从文件中读取的素数
uint32_t* s_primes = nullptr;
size_t prime_count = 0;
// 全局变量用于存储文件映射对象句柄
HANDLE hMapFile = NULL;

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

int32_t BigInteger::getLastDigit() const {
    return digits[0];
}

int32_t BigInteger::digitSum() const {
    int32_t sum = 0;
    for (int32_t digit : digits) {
        int32_t temp = digit;
        while (temp > 0) {
            sum += temp % 10;
            temp /= 10;
        }
    }
    return sum;
}

bool BigInteger::isLastDigitDivisibleBy2Or5() const {
    if (digitCount == 0) return false;
    int32_t lastDigit = digits[0] % 10;
    return lastDigit % 2 == 0 || lastDigit % 5 == 0;
}

bool BigInteger::isSpecialCase() const {
    if (*this < 2) {
        return false;
    }
    if (*this == 2 || *this == 3 || *this == 5) {
        return true;
    }
    if (isLastDigitDivisibleBy2Or5()) {
        return false;
    }
    if (digitSum() % 3 == 0) {
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
    return *this < BigInteger(other);
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
    BigInteger div;
	std::vector<BigInteger> primes;
    return isPrime(div,primes);
}

bool BigInteger::isPrime(BigInteger& divisor2, std::vector<BigInteger>& primes) const{
    if (isSpecialCase()) {
        return *this >= 2;
    }
    BigInteger limit = *this;
    for (const auto& prime : primes) {
        BigInteger primeSquared = prime * prime; // 预先计算 prime 的平方
        if (primeSquared > limit) {
            break;
        }
        if (*this % prime == BigInteger(0)) {
            divisor2 = prime;
            return false;
        }
    }
    BigInteger nextToCheck = primes.empty() ? BigInteger(3) : primes.back() + BigInteger(2);
        while (nextToCheck * nextToCheck <= *this) {
            bool isNewPrime = true;
            for (const auto& prime : primes) {
                BigInteger primeSquared = prime * prime; // 预先计算 prime 的平方
                if (primeSquared > nextToCheck) {
                    break;
                }
                if (nextToCheck % prime == BigInteger(0)) {
                    isNewPrime = false;
                    break;
                }
            }
            if (isNewPrime) {
                primes.push_back(nextToCheck);
                if (*this % nextToCheck == BigInteger(0)) {
                    return false;
                }
            }
            nextToCheck = nextToCheck + BigInteger(2);
        }
    return true;
}
    
// 判断是否为素数 
bool BigInteger::isPrime(int32_t* primes, int primeCount) const {
    if (isSpecialCase()) {
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

// 生成 int32_t 范围内从 7 开始的素数并保存到文件
/*void BigInteger::generatePrimesToFile(const char* filename, int32_t maxPrime) {
	std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    try {
        // 找到文件中最大的素数
        int32_t maxPrime = 7;
        std::ifstream readFile(filename);
        if (readFile.is_open()) {
            int32_t prime;
            while (readFile >> prime) {
                if (prime > maxPrime) {
                    maxPrime = prime;
                }
            }
            readFile.close();
        }
        // 从最大素数之后开始生成素数
        for (int32_t i = maxPrime + 1; i < std::numeric_limits<int32_t>::max(); ++i) {
            if (isPrime(i, PRECOMPUTED_PRIME_COUNT)) {
                file << i;
                file << ",";
            }
        }
    }catch (const std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	file.close();
}*/

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

bool BigInteger::readPrimesFromFile() {
	// 打开文件
	HANDLE hFile = CreateFileA("primes.txt", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to open file. Error code: " << errorCode << std::endl;
		return false;
	}
	// 获取文件大小
	LARGE_INTEGER fileSize;
	if (!GetFileSizeEx(hFile, &fileSize)) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to get file size. Error code: " << errorCode << std::endl;
		CloseHandle(hFile);
		return false;
	}
	// 创建文件映射对象
	HANDLE hMapFile = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hMapFile == NULL) {
		DWORD errorCode = GetLastError();
		std::cerr << "Failed to create file mapping. Error code: " << errorCode << std::endl;
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

bool BigInteger::is_prime(uint64_t value, uint64_t& divisor) noexcept
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

    if (value < std::numeric_limits<int32_t>::max()//4294967295
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
        ++step_index;
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
        ++step_index;
        step_index %= step_count;
    }

    return true;
}
