#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/error_code.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

int main()
{
	"generic error"_test = [] {
		should("error equals to the parent error") = [] {
			go::error_string eagain("Try again later");
			go::error err(eagain);

			expect(eagain == err);
			expect(err == eagain);
		};

		should("error doesn't equal to any concrete error other than parent") = [] {
			go::error_string eagain("Try again later");
			go::error err(eagain);

			go::error_string eagain2("Try again later");

			expect(eagain2 != err);
			expect(err != eagain2);
		};

		should("error equals to generic errors with the same parent") = [] {
			go::error_string eagain("Try again later");
			go::error err(eagain);
			go::error err2(eagain);

			expect(err == err2);
			expect(err2 == err);
		};

		should("error doesn't equal to any generic error with a different parent") = [] {
			go::error_string eagain("Try again later");
			go::error err(eagain);

			go::error_string eagain2("Try again later");
			go::error errOther(eagain2);

			expect(err != errOther);
			expect(errOther != err);
		};

		should("error.message() equals to the .message() of the parent") = [] {
			auto text = "Test error";
			go::error_string errConcrete(text);
			go::error err(errConcrete);

			expect(err.message() == errConcrete.message())
				<< "got" << err.message() << "want" << errConcrete.message();
		};

		should("error can be ostreamed") = [] {
			auto text = "Test error";
			go::error_string errConcrete(text);
			go::error err(errConcrete);

			std::stringstream ss;
			ss << err;

			expect(err.message() == ss.str())
				<< "got" << ss.str() << "want" << err.message();
		};

		should("default constructed error returns false in ifs") = [] {
			go::error err;

			expect(err == false);
		};

		should("errors with a value return true in ifs") = [] {
			go::error_string errStr("");
			go::error err(errStr);

			expect(err == true);
		};

        should("-> operator provides access to the error data's methods") = [] {
            auto stdCode = std::make_error_code(std::errc::operation_canceled);
            go::error_code errCode(stdCode);

            expect(errCode->code() == stdCode);
            expect(errCode->code().message() == stdCode.message());
            expect(errCode->value() == stdCode.value());
        };
	};

	return 0;
}
