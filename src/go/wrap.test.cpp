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

struct error_T_data : public go::error_interface, public go::is_interface<go::error>
{
	std::string s;

	explicit error_T_data(std::string s) : s(s) {}

	bool is(const go::error& other) const override
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

struct error_poser_data :
	public go::error_interface,
	public go::as_interface<error_T, error_fs_path>,
	public go::is_interface<go::error>
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

	bool is(const go::error& other) const override
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

struct error_poser_with_is_overload_data :
	public go::error_interface,
	public go::is_interface<error_wrapped, error_poser>
{
	std::string msg;

	error_poser_with_is_overload_data(std::string msg) :
		msg(std::move(msg))
	{}

	std::string message() const override
	{
		return msg;
	}

	bool is(const error_wrapped& other) const override
	{
		return true;
	}

	bool is(const error_poser& other) const override
	{
		return true;
	}
};

using error_poser_with_is_overload = go::error_of<error_poser_with_is_overload_data>;

struct error_pure_wrapped_data : public go::error_interface
{
	go::error err;

    error_pure_wrapped_data(go::error err) : err(std::move(err)) {}

	std::string message() const override { return "pure_wrapped"; }
	go::error unwrap() const override { return err; }
};

using error_pure_wrapped = go::error_of<error_pure_wrapped_data>;

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

template <class Target>
void asValidationTestCase(go::error err, Target&& value, const char* valueType)
{
	try
	{
		expect(!go::as_error(err, std::forward<Target>(value))) << "got as_error(err, " << valueType << ") = true, want false";
	}
	catch (...)
	{
		expect(false) << "as_error threw an exception on " << valueType << "value type";
	}
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

		auto poser2 = error_poser_with_is_overload("either wrapped or poser");

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

		// Duplicated because these are just 3 special cases
		should("either wrapped or poser is wrapped") = [&]
		{
			bool got = go::is_error(poser2, erra);
			expect(got == true)
				<< "is(" << poser2 << "," << erra << ") =" << got << ", want" << "true";
		};

		should("either wrapped or poser is poser") = [&]
		{
			bool got = go::is_error(poser2, poser);
			expect(got == true)
				<< "is(" << poser2 << "," << poser << ") =" << got << ", want" << "true";
		};

		should("either wrapped or poser is 1") = [&]
		{
			bool got = go::is_error(poser2, err1);
			expect(got == false)
				<< "is(" << poser2 << "," << err1 << ") =" << got << ", want" << "false";
		};
	};

	"as"_test = []
	{
		auto nilPoserFn = [](go::error) -> bool { std::terminate(); return false; };
		can_timeout* timeout;
		void* any;
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
		asTestCase<void*>(errF, true, errF);
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

	"as validation"_test = []
	{
		auto err = go::errorf("error");

		std::string str;
		char* ch = new char[6]("ababa");

		// Won't compile not an lvalue
		//asValidationTestCase(err, static_cast<void*>(nullptr), "void*");

		// Won't compile, not viable for dynamic cast
		//asValidationTestCase(err, (int*)nullptr, "int*");
		//asValidationTestCase(err, ch, "const char*");

		// Won't compile, string is not convertible to bool
		//asValidationTestCase(err, str, "std::string");

		struct { operator bool() { return true; } }* omg{};
		asValidationTestCase(err, omg, "OMG type");
	};

	"unwrap"_test = []
	{
		should("multi unwrapping nil error returns empty array") = []
		{
			go::error err;
			auto& unwrapped = err.unwrap_multiple();

			expect(unwrapped.size() == 0) << "got" << unwrapped.size() << " unwrapped errors, want 0";
		};

		auto err1 = go::errorf("1");
		auto erra = error_wrapped{ "wrap 2", err1 };

		struct testCase
		{
			go::error err, want;
		};

		std::vector<testCase> testCases{
			{go::error(), go::error()},
			{error_wrapped{"wrapped", go::error()}, go::error()},
			{err1, go::error()},
			{erra, err1},
			{error_wrapped{"wrap 3", erra}, erra},
		};

		for (auto&& tc : testCases)
		{
			std::stringstream ss;
			ss << "Unwrap(" << tc.err << ") = " << tc.want;

			should(ss.str().c_str()) = [&]
			{
				auto unwrapped = tc.err.unwrap();
				expect(unwrapped == tc.want);
			};
		}
	};

	"pure_wrapped"_test = []
	{
		auto err1 = go::errorf("error");
		auto err2 = error_pure_wrapped(err1);
		go::error errErased = err2;

		auto errUnwrapped = errErased.unwrap();

		expect(errUnwrapped != false) << "got nil unwrapped error, want non-nil";

		expect(err1 == errUnwrapped) << "got unwrapped error differs from the original, want to be the same";
	};

	return 0;
}