include_directories(../include)
add_executable(BigInt_Test
	Main.cpp)

set(CMAKE_CXX_STANDARD 20)


add_executable(print_primes print_primes.cpp)
add_executable(windows_mmfile_example windows_mmfile_example.cpp)
add_executable(is_prime is_prime.cpp)

target_link_libraries(BigInt_Test BigInt)
