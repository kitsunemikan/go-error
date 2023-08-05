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
    };

    return 0;
}

