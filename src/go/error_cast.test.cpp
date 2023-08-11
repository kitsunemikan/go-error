#include <go/error.hpp>
#include <go/error_cast.hpp>
#include <go/error_string.hpp>
#include <go/error_code.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

struct my_interface
{
	virtual void my_func() = 0;
};

struct error_my_1_data : public go::error_interface, public my_interface
{
	std::string message() const override { return "my_error_1"; }
	void my_func() override {}
};

struct error_my_2_data : public error_my_1_data
{
	static inline const char* msg = "my_error_2";

	// TODO: hack until #14 is fixed
	error_my_2_data(bool v) {};
	std::string message() const override { return msg; }
};

using error_my_1 = go::error_of<error_my_1_data>;
using error_my_2 = go::error_of<error_my_2_data>;

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

		should("cast to an intermediate type in the inheritance hierarchy succeeds") = [] {
			error_my_2 errMy2(true);
			go::error err(errMy2);

			auto errMy2Cast = go::error_cast<error_my_2>(err);
			expect(errMy2Cast == true) << "got cast to parent type failure, want success";
			expect(errMy2Cast == errMy2) << "casted error differs from original at address level";

			auto errMy1Cast = go::error_cast<error_my_1>(err);
			expect(errMy1Cast == true) << "got cast to intermediate type failure, want success";
			expect(errMy1Cast == errMy2) << "intermediate cast differs from parent at address level";
			expect(errMy1Cast.message() == error_my_2::impl_type::msg)
				<< "got msg from a casted intermediate error:" << errMy1Cast.message() << "want equal to the parent msg" << error_my_2::impl_type::msg;
		};

		should("cast to a raw impl pointer in hierarchy succeeds") = [] {
			error_my_2 errMy2(true);
			go::error err(errMy2);

			error_my_2_data* rawImpl2 = go::error_cast<error_my_2_data*>(err);
			expect(rawImpl2 == errMy2.data().get()) << "casted impl ptr differs from actual impl ptr";

			error_my_1_data* rawImpl1 = go::error_cast<error_my_1_data*>(err);
			expect(rawImpl1 == errMy2.data().get()) << "casted intermediate impl ptr differs from actual impl ptr";
		};

		should("cast to a raw arbitrary base pointer") = [] {
			auto data = std::make_shared<error_my_2_data>(true);
			auto rawWant = dynamic_cast<my_interface*>(data.get());

			expect(rawWant != nullptr) << "couldn't cast data pointer to my_interface, test assertion failed";

			error_my_2 errMy2(data);
			go::error err(errMy2);

			my_interface* rawGot = go::error_cast<my_interface*>(err);
			expect(rawGot == rawWant) << "raw pointers differ";
		};
	};

	return 0;
}
