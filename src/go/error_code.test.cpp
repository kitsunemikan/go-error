#include <go/error_code.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

int main()
{
	"error_code"_test = [] {
		should("error_code from an existing std::error_code") = [] {
			auto ec = std::make_error_code(std::errc::operation_canceled);
			go::error_code errEc(ec);

			expect(errEc.data()->code() == ec)
				<< "got error code" << errEc.data()->code().value() << "want" << ec.value();

			expect(errEc.data()->value() == ec.value())
				<< "got error code value" << errEc.data()->code().value() << "want" << ec.value();

			expect(errEc.data()->code().message() == ec.message())
				<< "got error code msg" << errEc.data()->code().message() << "want" << ec.message();
		};

		/*
		if (auto ecErr3 = go::error_cast<go::error_code>(ecErr2))
		{
			std::cout << "ecErr3 == ec : " << (ecErr3.data()->code() == ec) << '\n';
			std::cout << "ecErr3 == ecErr : " << (ecErr3 == ecErr) << '\n';
			std::cout << "ecErr3 == ecErr2 : " << (ecErr3 == ecErr2) << '\n';
		}
		else
		{
			std::cout << "Couldn't cast error back to error_code\n";
		}*/
	};
	return 0;
}
