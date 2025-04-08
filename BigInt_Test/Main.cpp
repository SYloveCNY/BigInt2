#include "BigIntegerDll.h"
#include <chrono>

// 全局变量用于存储从文件中读取的素数
uint32_t* s_primes = nullptr;
size_t prime_count = 0;
// 全局变量用于存储文件映射对象句柄
HANDLE hMapFile = NULL;

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

static bool searchValueInPrimes(const std::vector<uint32_t>& primes, uint32_t value) {
    if (value < 4294967295) {
        //std::numeric_limits<uint32_t>::max()) {
        return std::ranges::binary_search(primes, value);
    }
    return false;
}

bool readPrimesFromFile() {
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
    s_primes = static_cast<uint32_t*>(MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 0));
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

bool is_prime(uint64_t value, uint64_t& divisor) noexcept
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

 int main() { 

     if (!readPrimesFromFile()) {
         return 1;
     }

     uint64_t value = std::numeric_limits<uint64_t>::max();
     value -= 14;

     uint64_t divisor = 0;
     bool ret = is_prime(value, divisor);
     if (ret)
         std::cout << value << ", 是素数。" << std::endl;
     else
         std::cout << value << ", 可以被 " << divisor << " 整除。" << std::endl;

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

   std::cout << "计算 fib(" << n << "), 耗时: " << fib_diff << std::endl;  
   std::cout << "计算 " << x << "!, 耗时: " << fac_diff << std::endl; 

   std::cin.get();  

   // 解除映射
   UnmapViewOfFile(s_primes);
   // 关闭文件映射对象
   CloseHandle(hMapFile);

   return 0;  
}
