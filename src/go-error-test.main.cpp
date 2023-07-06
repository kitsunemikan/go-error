#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/errorf.hpp>
#include <go/error_code.hpp>

#include <iostream>
#include <sstream>

void load_images(go::error& err)
{
	err = go::errorf("open 'sprite.png': file doesn't exist");
}

void load_game(go::error& err)
{
	err = go::error();

	load_images(err);
	if (err)
	{
		err = go::errorf("load images: ", err);
		return;
	}
}

go::error load_images2()
{
	return go::errorf("open 'sprite.png': file doesn't exist");
}

go::error load_game2()
{
	auto imgErr = load_images2();
	if (imgErr)
	{
		return go::errorf("load images: ", imgErr);
	}

	return {};
}

int main()
{
	//
	// Features:
	// 1. Compare error (same ptr)
	// 2. Unwrap (as, is)
	// 3. Multierror

	go::error_string eagain("Try again later");
	go::error_string eof("End of file");
	go::error errEagain1(eagain);
	go::error errEagain2(eagain);

	std::cout << std::boolalpha;

	std::cout << "err1 == eagain: " << (errEagain1 == eagain) << '\n';
	std::cout << "err1 == eof: " << (errEagain1 == eof) << '\n';
	std::cout << "err1 == err2: " << (errEagain1 == errEagain2) << '\n';

	std::cout << "erragain = " << eagain << '\n';
	std::cout << "err1 = " << errEagain1 << '\n';

	go::error customErr = go::errorf("receive frame: ", eagain);
	std::cout << "customErr = " << customErr.message() << '\n';
	std::cout << "customErr == err1 : " << (customErr == errEagain1) << '\n';

	go::error gameErr;
	load_game(gameErr);
	if (gameErr)
	{
		std::cout << "error: load game: " << gameErr << '\n';
	}
	else
	{
		std::cout << "no game error!" << '\n';
	}

	auto gameErr2 = load_game2();
	if (gameErr2)
	{
		std::cout << "error: load game: " << gameErr2 << '\n';
	}
	else
	{
		std::cout << "no game error!" << '\n';
	}


	auto ec = std::make_error_code(std::errc::operation_canceled);
	go::error_code ecErr(ec);
	go::error ecErr2(ecErr);

	std::cout << "ecErr == ecErr2 : " << (ecErr == ecErr2) << '\n';
	std::cout << "ecErr == ec : " << (ecErr.data()->code() == ec) << '\n';


	return 0;
}