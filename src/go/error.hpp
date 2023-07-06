#pragma once

#include <type_traits>
#include <string>
#include <memory>

namespace go
{
	struct error_interface
	{
		virtual std::string message() const = 0;
		virtual ~error_interface() noexcept = default;
	};

	struct errorlike
	{
		virtual std::shared_ptr<error_interface> bare_error() const = 0;
		virtual std::string message() const = 0;
		virtual ~errorlike() noexcept = default;

		operator bool()
		{
			return bare_error().get() != nullptr;
		}
	};

	struct error : public errorlike
	{
		error() = default;

		template<
			class Error,
			class = std::enable_if<std::is_base_of_v<errorlike, Error>>::type
		>
		error(const Error& err)
		{
			err_ = err.bare_error();
		}

		std::shared_ptr<error_interface> bare_error() const override
		{
			return err_;
		}

		std::string message() const override
		{
			return err_->message();
		}

	private:
		std::shared_ptr<error_interface> err_;
	};
}

template<
	class ErrorA,
	class ErrorB,
	class = std::enable_if<
		std::is_base_of_v<go::errorlike, ErrorA> && std::is_base_of_v<go::errorlike, ErrorB>
	>::type
>
bool operator==(const ErrorA& a, const ErrorB& b)
{
	return a.bare_error().get() == b.bare_error().get();
}

template<
	class ErrorA,
	class ErrorB,
	class = std::enable_if<
		std::is_base_of_v<go::errorlike, ErrorA> && std::is_base_of_v<go::errorlike, ErrorB>
	>::type
>
bool operator!=(const ErrorA& a, const ErrorB& b)
{
	return !(a == b);
}

template <
	class Error,
	class = std::enable_if<std::is_base_of_v<go::errorlike, Error>>::type
>
std::ostream& operator<<(std::ostream& os, const Error& err)
{
	return os << err.message();
}

