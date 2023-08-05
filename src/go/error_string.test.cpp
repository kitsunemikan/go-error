#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/error_cast.hpp>
#include <go/error_code.hpp>
#include <go/errorf.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

#include <string>

int main()
{
    "error_string"_test = [] {
		should("error_strings with different contents differ") = [&] {
			go::error_string eagain("Try again later");
			go::error_string eof("End of file");

			expect(eagain != eof);
		};

		should("different error_strings with the same content equal") = [] {
			go::error_string eagain1("Try again later");
			go::error_string eagain2("Try again later");

			expect(eagain1 != eagain2);
		};

		should(".message() returns the message, error was created with") = [] {
			auto text = "Test error";
			go::error_string err(text);

			expect(text == err.message()) << "got" << err.message() << "want" << text;
		};

		should("error_string can be initialized with const char*") = [] {
			const char* text = "Test error";
			go::error_string err(text);

			expect(text == err.message()) << "got" << err.message() << "want" << text;
		};

		should("error_string can be initialized with std::string") = [] {
			std::string text = "Test error";
			go::error_string err(text);

			expect(text == err.message()) << "got" << err.message() << "want" << text;
		};

		should("error_string can be ostreamed") = [] {
			std::string text = "Test error";
			go::error_string err(text);

			std::stringstream ss;
			ss << err;

			expect(text == ss.str()) << "got" << ss.str() << "want" << text;
		};

		should("error_string default constructor inits with empty message") = [] {
			go::error_string err{};

			expect(err.message() == "") << "got" << err.message() << "want empty string";
		};
    };

	auto ec = std::make_error_code(std::errc::operation_canceled);
	go::error_code ecErr(ec);
	go::error ecErr2(ecErr);

	std::cout << "ecErr == ecErr2 : " << (ecErr == ecErr2) << '\n';
	std::cout << "ecErr == ec : " << (ecErr.data()->code() == ec) << '\n';

	if (auto ecErr3 = go::error_cast<go::error_code>(ecErr2))
	{
		std::cout << "ecErr3 == ec : " << (ecErr3.data()->code() == ec) << '\n';
		std::cout << "ecErr3 == ecErr : " << (ecErr3 == ecErr) << '\n';
		std::cout << "ecErr3 == ecErr2 : " << (ecErr3 == ecErr2) << '\n';
	}
	else
	{
		std::cout << "Couldn't cast error back to error_code\n";
	}

    return 0;
}

