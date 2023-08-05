#include <go/error.hpp>
#include <go/error_cast.hpp>
#include <go/error_string.hpp>
#include <go/error_code.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

namespace impl
{
	struct error_my_1 : public go::error_interface
	{
		std::string message() const override { return "my_error_1"; }
	};

	struct error_my_2 : public error_my_1
	{
		static inline const char* msg = "my_error_2";

		// TODO: hack until #14 is fixed
		error_my_2(bool v) {};
		std::string message() const override { return msg; }
	};
}

using error_my_1 = go::error_of<impl::error_my_1>;
using error_my_2 = go::error_of<impl::error_my_2>;

int main()
{
	"error_cast"_test = [] {
		auto test = "test";
		go::error_string errStr(test);
		go::error err(errStr);
		
		should("cast to the parent type succeeds") = [&] {
			auto errCastedOk = go::error_cast<go::error_string>(err);
			expect(errCastedOk == true) << "got cast failure, want successful cast to the parent error type";

			if (errCastedOk)
			{
				expect(errCastedOk == errStr) << "got parent error and casted error unequal, want equal";
				expect(err == errCastedOk) << "got casted and generic errors unequal, want equal";
			}
		};

		should("cast to a non parent type fails") = [&] {
			auto errCastedBad = go::error_cast<go::error_code>(err);
			expect(errCastedBad == false) << "got a valid error from an incompatible cast, want null error";
		};

		should("cast to an intermediate type in the inheritance hierarchy succeeds") = [&] {
			error_my_2 errMy2(true);
			go::error err(errMy2);

			auto errMy2Cast = go::error_cast<error_my_2>(err);
			expect(errMy2Cast == true) << "got cast to parent type failure, want success";

			auto errMy1Cast = go::error_cast<error_my_1>(err);
			expect(errMy1Cast == true) << "got cat to intermediate type failure, want success";
			expect(errMy1Cast.message() == error_my_2::impl_type::msg)
				<< "got msg from a casted intermediate error:" << errMy1Cast.message() << "want equal to the parent msg" << error_my_2::impl_type::msg;
		};
	};

	return 0;
}
