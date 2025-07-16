#include "BigInteger.h"

const int BigInteger::STEP[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
const int BigInteger::STEP_COUNT = sizeof(BigInteger::STEP) / sizeof(BigInteger::STEP[0]);
const int64_t BigInteger::BASE = 10'0000'0000LL;
const int BigInteger::DIGIT_WIDTH = 9;

uint32_t* BigInteger::sPrimes = nullptr;
size_t BigInteger::primeCount = 0;
MemoryMapFile BigInteger::sMemFile;


BigInteger::BigInteger(uint64_t num) {
	do {
		digits.push_back(num % BASE);
		num /= BASE;
	} while (num > 0);
	removeLeadingZeros();
}

// 检查所有块是否为零
bool BigInteger::allZero(const std::vector<uint64_t>& blocks) {
	for (uint64_t block : blocks) {
		if (block != 0) return false;
	}
	return true;
}

int BigInteger::compareDigitsAbsolute(const BigInteger& other) const {
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

int32_t BigInteger::sumOfDigits() const {
	int64_t sum = 0;
	for (int32_t digit : digits) {
		sum += digit;
		sum %= 3;  // 累加过程中对 3 取模，避免溢出
	}
	return sum;
}

bool BigInteger::isLastDigitDivisibleBy2Or5() const {
	if (digitCount == 0) return false;
	int32_t last_digit = digits[0];
	return last_digit % 2 == 0 || last_digit % 5 == 0;
}

bool BigInteger::isSpecialCase(BigInteger& divisor) const {
	if (*this == 2 || *this == 3 || *this == 5) {
		return true;
	}
	if (isLastDigitDivisibleBy2Or5()) {
		divisor = 2;
		if (digits[0] == 5) {
			divisor = 5;
		}
		return true;
	}
	if (sumOfDigits() % 3 == 0) {
		divisor = 3;
		return true;
	}
	return false;
}

auto BigInteger::compareDigits(const BigInteger& other) const {
	if (digits.size() < other.digits.size())
		return std::strong_ordering::less;
	else if (digits.size() > other.digits.size())
		return std::strong_ordering::greater;

	for (int i = digits.size() - 1; i >= 0; --i) {
		if (digits[i] < other.digits[i])
			return std::strong_ordering::less;
		else if (digits[i] > other.digits[i])
			return std::strong_ordering::greater;
	}

	return std::strong_ordering::equal;
}

bool BigInteger::checkPrimeWithStep(BigInteger& divisor, BigInteger start) const {
	int stepIndex = 0;
	BigInteger x = start;

	while (true) {
		// 一次性计算商和余数
		auto [quotient, remainder] = this->innerDiv(x);

		// 检查整除性
		if (remainder == 0) {
			divisor = x;
			return false;
		}

		// 提前终止条件：如果商小于等于当前除数，后续不可能整除
		if (quotient <= x) {
			break;
		}

		x = BigInteger(STEP[stepIndex]) + x;
		++stepIndex;
		stepIndex %= STEP_COUNT;
	}
	return true;
}

BigInteger BigInteger::innerAdd(const BigInteger& other) const {
	BigInteger result;
	result.digits.clear();
	result.digits.reserve(std::max(digits.size(), other.digits.size()) + 1);
	int64_t carry = 0;
	size_t common_size = std::min(digits.size(), other.digits.size());

	// 处理共同的位
	for (size_t i = 0; i < common_size; ++i) {
		carry += digits[i] + other.digits[i];
		result.digits.push_back(carry % BASE);
		carry /= BASE;
	}

	// 处理第一个大整数剩余的位
	if (digits.size() > common_size) {
		for (size_t i = common_size; i < digits.size(); ++i) {
			carry += digits[i];
			result.digits.push_back(carry % BASE);
			carry /= BASE;
		}
	}
	// 处理第二个大整数剩余的位
	else if (other.digits.size() > common_size) {
		for (size_t i = common_size; i < other.digits.size(); ++i) {
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

BigInteger BigInteger::innerSub(const BigInteger& other) const {
	if (compareDigits(other) == std::strong_ordering::less) {
		return other.innerSub(*this);
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

BigInteger BigInteger::innerMul(const BigInteger& other) const {
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

std::pair<BigInteger, BigInteger> BigInteger::innerDiv(const BigInteger& divisor) const {
	if (divisor.digits.size() > 1) {
		bool is_power_of_ten = true;
		for (size_t i = 0; i < divisor.digits.size() - 1; ++i) {
			if (divisor.digits[i] != 0) {
				is_power_of_ten = false;
				break;
			}
		}
		if (is_power_of_ten && divisor.digits.back() == 1) {
			BigInteger quotient;
			BigInteger remainder;
			size_t shift = divisor.digits.size() - 1;
			if (shift >= digits.size()) {
				quotient.digits = { 0 };
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

		if (remainder.compareDigits(divisor) == std::strong_ordering::less) {
			quotient.digits.insert(quotient.digits.begin(), 0);
			continue;
		}

		int64_t left = 0, right = BASE - 1;
		int64_t q = 0;
		while (left <= right) {
			int64_t mid = left + ((right - left) >> 1); // 使用位运算代替除法
			BigInteger temp = divisor * mid;
			if (temp.compareDigits(remainder) != std::strong_ordering::greater) {
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

auto BigInteger::operator<=>(const BigInteger& other) const {
	if (isNegative != other.isNegative) {
		return isNegative ? std::strong_ordering::less : std::strong_ordering::greater;
	}

	auto ret = compareDigits(other);
	if (isNegative && ret != std::strong_ordering::equal) {
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
	bool is_negative_other = other < 0;
	if (isNegative != is_negative_other)
		return false;

	int i = 0;
	int64_t absolute_other = is_negative_other ? -other : other;
	std::lldiv_t tmp = std::lldiv(absolute_other, BASE);
	while (tmp.quot > 0) {
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
		BigInteger abs_other(-other);
		return compareDigitsAbsolute(abs_other) > 0;
	}
	else {
		if (other < 0) {
			return false;  // 正数大于负数
		}
		// 两个正数比较
		BigInteger abs_other(other);
		return compareDigitsAbsolute(abs_other) < 0;
	}
}

bool BigInteger::operator<=(const BigInteger& other) const {
	// 实现比较逻辑，这里简单假设 digits 存储大整数的每一位
	if (digits.size() < other.digits.size()) {
		return true;
	}
	else if (digits.size() > other.digits.size()) {
		return false;
	}
	for (int i = digits.size() - 1; i >= 0; --i) {
		if (digits[i] < other.digits[i]) {
			return true;
		}
		else if (digits[i] > other.digits[i]) {
			return false;
		}
	}
	return true;
}

BigInteger BigInteger::operator+(const BigInteger& other) const {
	if (isNegative == other.isNegative) {
		BigInteger result = innerAdd(other);
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
		if (compareDigits(other) == std::strong_ordering::less) {
			BigInteger result = other.innerSub(*this);
			result.isNegative = !isNegative;
			return result;
		}
		BigInteger result = innerSub(other);
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
	BigInteger result = innerMul(other);
	result.isNegative = isNegative != other.isNegative;
	return result;
}

BigInteger BigInteger::operator/(const BigInteger& other) const {
	if (other.isZero()) {
		throw std::invalid_argument("Division by zero");
	}
	BigInteger abs_dividend = *this;
	abs_dividend.isNegative = false;
	BigInteger abs_divisor = other;
	abs_divisor.isNegative = false;
	auto [quotient, _] = abs_dividend.innerDiv(abs_divisor);
	quotient.isNegative = isNegative != other.isNegative;
	return quotient;
}

BigInteger BigInteger::operator%(const BigInteger& other) const {
	if (other.isZero()) {
		throw std::invalid_argument("Modulo by zero");
	}
	BigInteger abs_dividend = *this;
	abs_dividend.isNegative = false;
	BigInteger abs_divisor = other;
	abs_divisor.isNegative = false;
	auto [_, remainder] = abs_dividend.innerDiv(abs_divisor);
	remainder.isNegative = isNegative;
	return remainder;
}

bool BigInteger::isPrimeNumber() const {
	BigInteger divisor;
	return isPrimeNumber(divisor);
}

bool BigInteger::isPrimeNumber(BigInteger& divisor) const noexcept {
	//// 特殊情况
	//if (*this < 2) return false;
	//if (isSpecialCase(divisor)) {
	//	return true;
	//}
	//// 尝试加载素数文件
	//if (!sPrimes) { // 仅在首次调用时加载
	//	size_t file_size = 0;
	//	sPrimes = static_cast<uint32_t*>(sMemFile.loadFile(L"primes.dat", file_size));
	//	if (sPrimes) {
	//		primeCount = file_size / sizeof(uint32_t);
	//	}
	//}
	//// 使用素数文件进行快速判断
	//if (sPrimes) {
	//	if (*this < std::numeric_limits<uint32_t>::max()
	//		&& std::ranges::binary_search(sPrimes, sPrimes + primeCount, *this)) {
	//		return true;
	//	}

	//	for (size_t i = 0; i < primeCount; ++i) {
	//		BigInteger x = sPrimes[i];
	//		if (x * x > *this) {
	//			break;
	//		}
	//		if ((*this % x) == 0) {
	//			divisor = x;
	//			return false;
	//		}
	//	}

	//	uint32_t last_prime = sPrimes[primeCount - 1];

	//	int v = (last_prime - 7) % 30;
	//	int step_index = 0;
	//	while (v > 0) {
	//		v -= STEP[step_index];
	//		++step_index;
	//		step_index %= STEP_COUNT;
	//	}

	//	BigInteger start = last_prime;
	//	return checkPrimeWithStep(divisor, start);
	//}
	// 素数文件加载失败，从 7 开始判断   
	BigInteger start = 7;
	return checkPrimeWithStep(divisor, start);
}

BigInteger BigInteger::fibonacci(int64_t n) {
	if (n == 0) return BigInteger(0);
	if (n == 1) return BigInteger(1);

	Matrix base;
	base.data[0][0] = BigInteger(1);
	base.data[0][1] = BigInteger(1);
	base.data[1][0] = BigInteger(1);
	base.data[1][1] = BigInteger(0);

	Matrix result = base.fastPower(n - 1);
	return result.data[0][0];
}

BigInteger BigInteger::factorial(int64_t n) {
	if (n < 0) {
		throw std::invalid_argument("Factorial is not defined for negative numbers.");
	}
	BigInteger result(1);
	for (int i = 2; i <= n; ++i) {
		result = result * BigInteger(i);
	}
	return result;
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

BIGINTEGER_DLL_API Matrix::Matrix() {
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			data[i][j] = BigInteger(0);
		}
	}
}

BIGINTEGER_DLL_API Matrix Matrix::operator*(const Matrix& other) const {
	Matrix result;
	for (int i = 0; i < 2; ++i) {
		for (int j = 0; j < 2; ++j) {
			for (int k = 0; k < 2; ++k) {
				result.data[i][j] = result.data[i][j] + data[i][k] * other.data[k][j];
			}
		}
	}
	return result;
}

BIGINTEGER_DLL_API Matrix Matrix::fastPower(int n) const {
	Matrix result;
	result.data[0][0] = BigInteger(1);
	result.data[1][1] = BigInteger(1);
	Matrix temp = *this;
	while (n > 0) {
		if (n & 1) {
			result = result * temp;
		}
		temp = temp * temp;
		n >>= 1;
	}
	return result;
}

BIGINTEGER_DLL_API BigInteger operator"" _bi(const char* str, size_t len) {
	// 检查合法性：分隔符不能出现在首尾，且不能连续出现
	if (len > 0) {
		if (str[0] == '\'' || str[len - 1] == '\'') {
			throw std::invalid_argument("BigInteger字面量首尾不能使用分隔符");
		}
		for (size_t i = 0; i < len - 1; i++) {
			if (str[i] == '\'' && str[i + 1] == '\'') {
				throw std::invalid_argument("BigInteger字面量不能包含连续的分隔符");
			}
		}
	}

	// 移除所有分隔符并处理符号
	std::string cleanStr;
	bool isNegative = false;
	bool hasSign = false;
	bool hasNonSignChar = false;  // 标记是否已出现非符号字符

	for (size_t i = 0; i < len; ++i) {
		if (str[i] == '\'') continue;

		// 检查是否为符号位
		if (!hasSign && (str[i] == '+' || str[i] == '-')) {
			// 符号位必须是第一个有效字符（不能在非符号字符之后）
			if (hasNonSignChar) {
				throw std::invalid_argument("符号位只能出现在首位");
			}
			hasSign = true;
			if (str[i] == '-') isNegative = true;
			continue;
		}

		// 标记已出现非符号字符
		hasNonSignChar = true;

		// 确保符号后紧跟数字
		if (hasSign && cleanStr.empty() && !std::isdigit(str[i])) {
			throw std::invalid_argument("符号后必须紧跟数字");
		}

		if (!std::isdigit(str[i])) {
			throw std::invalid_argument("BigInteger字面量包含非法字符");
		}

		cleanStr += str[i];
	}

	// 处理空字符串或全零
	if (cleanStr.empty() || cleanStr == "0") {
		return BigInteger({}, false);  // 零值
	}

	// 移除前导零
	size_t firstNonZero = cleanStr.find_first_not_of('0');
	if (firstNonZero != std::string::npos) {
		cleanStr = cleanStr.substr(firstNonZero);
	}
	else {
		cleanStr = "0"; // 如果全是零，设置为 "0"
	}

	// 直接构造 digits 向量（低位在前）
	std::vector<int32_t> digits;

	// 从低位到高位，按每 9 位一组处理
	for (size_t i = cleanStr.length(); i > 0; ) {
		// 计算当前组的起始位置（确保不越界）
		size_t start = (i >= BigInteger::DIGIT_WIDTH) ? (i - BigInteger::DIGIT_WIDTH) : 0;

		// 计算当前组的长度（确保不越界）
		size_t length = i - start;

		std::string group = cleanStr.substr(start, length);

		// 使用 stoll 避免溢出，并检查范围
		int32_t value = std::stoi(group);

		digits.push_back(static_cast<int32_t>(value));

		// 更新循环变量（直接跳到下一组）
		i = start;
	}

	// 使用私有构造函数直接构造 BigInteger
	return BigInteger(std::move(digits), isNegative);
}