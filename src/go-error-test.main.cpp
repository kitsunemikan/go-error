#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/errorf.hpp>

#include <iostream>

int main()
{
	//
	// Features:
	// 1. Compare error (same ptr)

	go::error_string eagain("Try again later");
	go::error_string eof("End of file");
	go::error errEagain1(eagain);
	go::error errEagain2(eagain);

	std::cout << std::boolalpha;

	std::cout << "err1 == eagain: " << (errEagain1 == eagain) << '\n';
	std::cout << "err1 == eof: " << (errEagain1 == eof) << '\n';
	std::cout << "err1 == err2: " << (errEagain1 == errEagain2) << '\n';

	return 0;
}