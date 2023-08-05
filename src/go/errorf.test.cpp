#include <go/error.hpp>
#include <go/errorf.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

struct os_obj {
	static inline const char* msg = "custom ostream";
};

std::ostream& operator<<(std::ostream& os, const os_obj& obj)
{
	return os << os_obj::msg;
}

int main()
{
	"errorf"_test = [] {
		should("errorf with a single const char*") = [] {
			const char* text = "hello";
			auto err = go::errorf(text);

			expect(err.message() == text) << "got" << err.message() << "want" << text;
		};

		should("errorf with a single std::string") = [] {
			std::string text = "hello";
			auto err = go::errorf(text);

			expect(err.message() == text) << "got" << err.message() << "want" << text;
		};

		should("errorf with custom ostreamable object") = [] {
			auto err = go::errorf(os_obj());

			expect(err.message() == os_obj::msg) << "got" << err.message() << "want" << os_obj::msg;
		};

		should("errorf passes its arguments sequantially to an ostream") = [] {
			auto err = go::errorf("the error is:", os_obj(), 42);

			auto wantMsg = std::string("the error is:") + os_obj::msg + std::to_string(42);

			expect(err.message() == wantMsg) << "got" << err.message() << "want" << wantMsg;
		};

		should("errorf of the same arguments return unequal errors") = [] {
			auto err1 = go::errorf("test");
			auto err2 = go::errorf("test");

			expect(err1 != err2);
		};

		should("the same allocation should equal to itself (not crash)") = [] {
			auto err = go::errorf("test");

			expect(err == err);
		};
	};
	return 0;
}
