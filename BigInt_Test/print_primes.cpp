#include <iostream>
#include <deque>
#include <limits>
#include <fstream>
#include <memory>

static bool isPrime(uint32_t value, const uint32_t* primes, size_t size) noexcept
{
	for (size_t i = 0; i < size; ++i)
	{
		const uint32_t x = primes[i];
		if (value % x == 0)
			return false;
		if (value / x < x)
			break;
	}

	//	std::cout << value << ',';
	return true;
}

static void printPrimes() noexcept
{
	static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
	int stepCount = sizeof(step) / sizeof(step[0]);

	size_t primeSize = 0;
	std::unique_ptr<uint32_t[]> primes(new uint32_t[203280218]);
	//	std::vector<uint32_t> primes;
	//	primes.reserve(105097561);
	//	primes.reserve(203280218);
	//	std::deque<uint32_t> primes;
	int stepIndex = 0;//step_count - 1;
	uint64_t i = 7;
	while (i < std::numeric_limits<uint32_t>::max())
	{
		if (isPrime(i, primes.get(), primeSize))
			primes[primeSize++] = i;

		i += step[stepIndex];
		++stepIndex;
		stepIndex %= stepCount;
	}

	/*	std::vector<uint32_t> primes2;
		primes2.reserve(primes.size());
		primes2.assign(primes.cbegin(), primes.cend());
	*/	std::ofstream f("primes.dat", std::ios::binary);
	f.write((const char*)primes.get(), primeSize * sizeof(uint32_t));
	std::cout << "primes count: " << primeSize << std::endl;
	/*
		std::ofstream f("primes.txt");
		for (auto v: primes)
			f << v << ',' << std::endl;
	*/
}

int main(int argc, char* argv[])
{
	printPrimes();
	return 0;
};
