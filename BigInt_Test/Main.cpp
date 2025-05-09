﻿#include "BigInteger.h"
#include "MemoryMapFile.h"
#include <chrono>

class Matrix {
public:
    BigInteger data[2][2];

    Matrix() {
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                data[i][j] = BigInteger(0);
            }
        }
    }

    // 矩阵乘法
    Matrix operator*(const Matrix& other) const {
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
};

// 矩阵快速幂
static Matrix matrixPower(const Matrix& matrix, int n) {
    Matrix result;
    result.data[0][0] = BigInteger(1);
    result.data[1][1] = BigInteger(1);
    Matrix temp = matrix;
    while (n > 0) {
        if (n & 1) {
            result = result * temp;
        }
        temp = temp * temp;
        n >>= 1;
    }
    return result;
}

static BigInteger fibonacci(int64_t n) {
    if (n == 0) return BigInteger(0);
    if (n == 1) return BigInteger(1);

    Matrix base;
    base.data[0][0] = BigInteger(1);
    base.data[0][1] = BigInteger(1);
    base.data[1][0] = BigInteger(1);
    base.data[1][1] = BigInteger(0);

    Matrix result = matrixPower(base, n - 1);
    return result.data[0][0];
}

static BigInteger factorial(int64_t n) {
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

    const auto primes_start = std::chrono::steady_clock::now();
    BigInteger value = fibonacci(10006);
    std::cout << value;
    BigInteger divisor = 0;
    bool ret = value.isPrime(divisor); // 使用实例调用非静态成员函数
    if (ret)
        std::cout << value << ", 是素数。" << std::endl;
    else
        std::cout << value << ", 可以被 " << divisor << " 整除。" << std::endl;
    const auto primes_end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> primes_diff = primes_end - primes_start;

/*
    BigInteger value = std::numeric_limits<uint64_t>::max();
    value = value - 14;
    BigInteger divisor = 0;
    bool ret = value.isPrime(divisor); // 使用实例调用非静态成员函数
    if (ret)
        std::cout << value << ", 是素数。" << std::endl;
    else
        std::cout << value << ", 可以被 " << divisor << " 整除。" << std::endl;
*/
/*
   // 示例：判断 15 是否为素数
    BigInteger num = 15;
    BigInteger divisor = 0;
    bool numret = num.isPrime(divisor); // 使用实例调用非静态成员函数
    if (numret) {
        std::cout << num << " 是素数。" << std::endl;
    }
    else {
        std::cout << num << " 不是素数，能被 " << divisor << " 整除。" << std::endl;
    }
*/
    BigInteger num1(98'7654'3210);
    BigInteger num2(12'3456'7890);

    std::cout << "num1: " << num1 << std::endl;
    std::cout << "num2: " << num2 << std::endl;

    BigInteger sum = num1 + num2;
    std::cout << "Sum1: " << sum << std::endl;

    BigInteger diff = num1 - num2;
    std::cout << "Difference: " << diff << std::endl;

    BigInteger product = num1 * num2;
    std::cout << "Product: " << product << std::endl;

    BigInteger quotient = num1 / num2;
    std::cout << "Quotient: " << quotient << std::endl;

    BigInteger remainder = num1 % num2;
    std::cout << "Remainder: " << remainder << std::endl;

    const auto fib_start = std::chrono::steady_clock::now();
    int n = 10'0000;
    BigInteger fib = fibonacci(n);
    const auto fib_end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> fib_diff = fib_end - fib_start;

    std::cout << "Fibonacci(" << n << ") = " << fib << std::endl;

    const auto fac_start = std::chrono::steady_clock::now();
    int x = 1000;
    BigInteger fact = factorial(x);
    const auto fac_end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> fac_diff = fac_end - fac_start;

    std::cout << x << "! = " << fact << std::endl;

    std::cout << "判断素数fib（10006）耗时: " << primes_diff << std::endl;
    std::cout << "计算 fib(" << n << "), 耗时: " << fib_diff << std::endl;
    std::cout << "计算 " << x << "!, 耗时: " << fac_diff << std::endl;

    std::cin.get();

    return 0;
}
