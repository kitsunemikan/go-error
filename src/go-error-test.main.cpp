#include <go/error.hpp>

#include <iostream>

int main()
{
	go::error err("test");

	std::cout << err.message() << '\n';

	return 0;
}