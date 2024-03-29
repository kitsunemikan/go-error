#include <go/error.hpp>
#include <go/error_string.hpp>
#include <go/error_code.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

struct error_tag_data : public go::error_string_data
{
    error_tag_data() :
        go::error_string_data("tag")
    {}
};

using error_tag = go::error_of<error_tag_data>;

int main()
{
	"generic error"_test = [] {
		should("error equals to the parent error") = [] {
			auto eagain = go::make_error<go::error_string>("Try again later");
			go::error err(eagain);

			expect(eagain == err);
			expect(err == eagain);
		};

		should("error doesn't equal to any concrete error other than parent") = [] {
			auto eagain = go::make_error<go::error_string>("Try again later");
			go::error err(eagain);

			auto eagain2 = go::make_error<go::error_string>("Try again later");

			expect(eagain2 != err);
			expect(err != eagain2);
		};

		should("error equals to generic errors with the same parent") = [] {
			auto eagain = go::make_error<go::error_string>("Try again later");
			go::error err(eagain);
			go::error err2(eagain);

			expect(err == err2);
			expect(err2 == err);
		};

		should("error doesn't equal to any generic error with a different parent") = [] {
			auto eagain = go::make_error<go::error_string>("Try again later");
			go::error err(eagain);

			auto eagain2 = go::make_error<go::error_string>("Try again later");
			go::error errOther(eagain2);

			expect(err != errOther);
			expect(errOther != err);
		};

		should("error.message() equals to the .message() of the parent") = [] {
			auto text = "Test error";
			auto errConcrete = go::make_error<go::error_string>(text);
			go::error err(errConcrete);

			expect(err.message() == errConcrete.message())
				<< "got" << err.message() << "want" << errConcrete.message();
		};

		should("error can be ostreamed") = [] {
			auto text = "Test error";
			auto errConcrete = go::make_error<go::error_string>(text);
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
			auto errStr = go::make_error<go::error_string>("");
			go::error err(errStr);

			expect(err == true);
		};

        should("-> operator provides access to the error data's methods") = [] {
            auto stdCode = std::make_error_code(std::errc::operation_canceled);
            auto errCode = go::make_error<go::error_code>(stdCode);

            expect(errCode->code() == stdCode);
            expect(errCode->code().message() == stdCode.message());
            expect(errCode->value() == stdCode.value());
        };

        should("make_error with no arguments creates a non-empty error") = [] {

            auto errCtor = error_tag();
            auto errMade = go::make_error<error_tag>();

            expect(errCtor == false);
            expect(errMade == true);
            expect(errMade.message() == "tag");
        };
	};

	return 0;
}
