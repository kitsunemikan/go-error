#include <go/go_error.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

#include <string>
#include <vector>
#include <functional>
#include <fstream>
#include <filesystem>
#include <system_error>

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

struct error_T_data : public go::error_interface
{
	std::string s;

	explicit error_T_data(std::string s) : s(s) {}

	bool is(go::error other) const override
	{
		auto otherMe = go::error_cast<go::error_of<error_T_data>>(other);
		if (!otherMe)
			return false;

		return s == otherMe.data()->s;
	}

	std::string message() const override
	{
		return "error_T(" + s + ")";
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

struct can_timeout
{
	virtual bool timeout() = 0;
	virtual ~can_timeout() = default;
};

// PathError records an error and the operation and file path that caused it.
struct error_fs_path_data : public go::error_interface, public can_timeout
{
	std::string op, path;
	go::error err;

	error_fs_path_data() = default;
	~error_fs_path_data() noexcept = default;

	explicit error_fs_path_data(std::string op, std::string path = "", go::error err = {}) :
		op(op), path(path), err(err)
	{}

	std::string message() const override
	{
		return op + " " + path + ": " + err.message();
	}

	go::error unwrap() const override
	{
		return err;
	}

	bool timeout() override
	{
		auto timeoutErr = dynamic_cast<can_timeout*>(err.data().get());
		if (!timeoutErr)
			return false;

		return timeoutErr->timeout();
	}
};

using error_wrapped = go::error_of<error_wrapped_data>;
using error_multi = go::error_of<error_multi_data>;
using error_T = go::error_of<error_T_data>;
using error_fs_path = go::error_of<error_fs_path_data>;

auto poserPathErr = error_fs_path("poser");
auto poserErrT = error_T("poser");

struct error_poser_data : public go::error_interface, public go::as_interfaces<error_T, error_fs_path>
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

	// TODO: no access to the shared ptr of _data classes
	// so can't set other errors to ourselves.
	// Although it may be redundant

	void as(error_T& err) const override
	{
		err = poserErrT;
	}

	void as(error_fs_path& err) const override
	{
		err = poserPathErr;
	}
};

using error_poser = go::error_of<error_poser_data>;

std::pair<std::ifstream, go::error> openFile(std::filesystem::path name)
{
	if (name.empty())
	{
		auto ec = std::make_error_code(std::errc::no_such_file_or_directory);
		return { std::ifstream{}, error_fs_path("open", name.string(), go::error_code(ec))};
	}

	std::error_code ec;
	auto status = std::filesystem::status(name, ec);
	if (ec)
	{
		return { std::ifstream{}, error_fs_path("open: get status", name.string(), go::error_code(ec))};
	}

	return { std::ifstream(name.string()), go::error() };
}

template <class Target>
void asTestCase(go::error err, bool wantMatch, go::error wantTarget)
{
	std::stringstream ss;
	ss << err << " AS " << wantTarget << " (" << (wantMatch ? "ok" : "no") << ")";

	should(ss.str().c_str()) = [&]() {
		Target target;
		bool gotMatch = go::as_error(err, target);
		expect(gotMatch == wantMatch) << "got" << gotMatch << "want" << wantMatch;

		if (gotMatch == wantMatch && wantMatch == false)
			return;

		auto wantTargetTyped = go::error_cast<Target>(wantTarget);
		if (!wantTargetTyped)
			expect(false) << "couldn't cast test case to the desired concrete value";

		expect(target == wantTargetTyped) << "got" << target << "want" << wantTarget;
	};
}

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

	"as"_test = []
	{
		auto nilPoserFn = [](go::error) -> bool { std::terminate(); return false; };
		can_timeout* timeout;
		error_poser p;
		auto [_, errF] = openFile("non-existing");
		error_poser poserErr("oh no", nilPoserFn);

		error_T errA("a"), errB("b"), errT("T");

		asTestCase<error_fs_path>({}, false, {});
		asTestCase<error_T>(
			error_wrapped("pitied the fool", errT),
			true,
			errT);
		asTestCase<error_fs_path>(errF, true, errF);
		asTestCase<error_fs_path>(error_T(), false, {});
		asTestCase<error_T>(error_wrapped("wrapped", go::error()), false, {});
		asTestCase<error_T>(error_poser("error", nilPoserFn), true, poserErrT);
		asTestCase<error_fs_path>(error_poser("path", nilPoserFn), true, poserPathErr);
		asTestCase<error_poser>(poserErr, true, poserErr);
		asTestCase<can_timeout*>(go::errorf("err"), false, {});
		asTestCase<can_timeout*>(errF, true, errF);
		asTestCase<can_timeout*>(error_wrapped("path error", errF), true, errF);
		asTestCase<error_T>(error_multi(), false, {});
		asTestCase<error_T>(
			error_multi(go::errorf("a"), errT),
			true,
			errT);
		asTestCase<error_T>(
			error_multi(errT, go::errorf("a")),
			true,
			errT);
		asTestCase<error_T>(
			error_multi(errA, errB),
			true,
			errA);
		asTestCase<error_T>(
			error_multi(
				error_multi(go::errorf("a"), errA),
				errB),
			true,
			errA);
		asTestCase<can_timeout*>(
			error_multi(error_wrapped("path error", errF)),
			true,
			errF);
		asTestCase<error_T>(
			error_multi(go::error()),
			false,
			{});
	};

	return 0;
}