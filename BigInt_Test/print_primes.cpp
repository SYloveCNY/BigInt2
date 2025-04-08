#include <iostream>
#include <deque>
#include <limits>
#include <fstream>
#include <vector>

bool is_prime(uint32_t value, std::vector<uint32_t>& primes) noexcept
{
	for (auto x : primes)
	{
		if (value % x == 0)
			return false;
		if (value / x < x)
			break;
	}

	std::cout << value << ',';
	return true;
}

void print_primes() noexcept
{
	static const int step[] = { 4, 2, 4, 2, 4, 6, 2, 6, };
	int step_count = sizeof(step) / sizeof(step[0]);

	std::vector<uint32_t> primes;
	primes.reserve(105097561);
//	std::deque<uint32_t> primes;
	int step_index = step_count - 1;
	for (uint32_t i = 7; i < std::numeric_limits<int32_t>::max(); i += step[step_index])
	{
		if (is_prime(i, primes))
			primes.push_back(i);

		++ step_index;
		step_index %= step_count;
	}

	std::ofstream f("primes.dat", std::ios::binary);
	f.write((const char*)primes.data(), primes.size() * sizeof(uint32_t));
/*
	std::ofstream f("primes.txt");
	for (auto v: primes)
		f << v << ',' << std::endl;
*/
}

int main(int argc, char* argv[])
{
	print_primes();
	return 0;
};
