#include <go/go_error.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

#include <string>
#include <vector>
#include <functional>

struct error_wrapped_data : public go::error_interface
{
	std::string msg;
	go::error err;

	error_wrapped_data(std::string msg, go::error err) :
		msg(msg), err(err)
	{}

	std::string message() const override
	{
		return msg;
	}

	go::error unwrap() const override
	{
		return err;
	}
};

struct error_poser_data : public go::error_interface
{
	std::string msg;
	std::function<bool(go::error)> f;

	error_poser_data(std::string msg, std::function<bool(go::error)> f) :
		msg(msg), f(std::move(f))
	{}

	std::string message() const override
	{
		return msg;

	}

	bool is(go::error other) const override
	{
		return f(other);
	}
};

struct error_multi_data : public go::error_interface
{
	std::vector<go::error> errs;

	template <class... Errs>
	error_multi_data(Errs&&... errs) :
		errs{std::forward<Errs>(errs)...}
	{}

	std::string message() const override
	{
		std::stringstream ss;
		ss << "error_multierr{ ";
		for (auto& err : errs)
			ss << err << " ";
		ss << "}";

		return ss.str();
	}

	const std::vector<go::error>& unwrap_multiple() const
	{
		return errs;
	}
};

using error_wrapped = go::error_of<error_wrapped_data>;
using error_poser = go::error_of<error_poser_data>;
using error_multi = go::error_of<error_multi_data>;

int main()
{
	"is"_test = []
	{
		auto err1 = go::errorf("1");
		auto erra = error_wrapped("wrap 2", err1);
		auto errb = error_wrapped("wrap 3", erra);

		auto err3 = go::errorf("3");

		auto poser = error_poser("either 1 or 3", [&](go::error err)
		{
			return err == err1 || err == err3;
		});

		struct test_case
		{
			go::error err, target;
			bool match;
		};

		std::vector<test_case> testCases{
			{{}, {}, true},
			{err1, {}, false},
			{err1, err1, true},
			{erra, err1, true},
			{errb, err1, true},
			{err1, err3, false},
			{erra, err3, false},
			{errb, err3, false},
			{poser, err1, true},
			{poser, err3, true},
			{poser, erra, false},
			{poser, errb, false},
			{error_multi{}, err1, false},
			{error_multi{err1, err3}, err1, true},
			{error_multi{err3, err1}, err1, true},
			{error_multi{err1, err3}, go::errorf("x"), false},
			{error_multi{err3, errb}, errb, true},
			{error_multi{err3, errb}, erra, true},
			{error_multi{err3, errb}, err1, true},
			{error_multi{errb, err3}, err1, true},
			{error_multi{poser}, err1, true},
			{error_multi{poser}, err3, true},
			{error_multi{go::error{}}, {}, false},
		};

		for (auto& test : testCases)
		{
			std::stringstream ss;
			ss << test.err.message() << " " << (test.match ? "is" : "isn't") << " " << test.target.message();

			should(ss.str().c_str()) = [&]
			{
				bool got = go::is_error(test.err, test.target);
				expect(got == test.match)
					<< "is(" << test.err << "," << test.target << ") =" << got << ", want" << test.match;
			};
		}
	};

	return 0;
}