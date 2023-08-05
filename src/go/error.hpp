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

	template <class Impl>
	struct error_of
	{
		using impl_type = Impl;

				// TODO: is A base of A?
		template<
			class OtherImpl,
			class = std::enable_if<std::is_base_of_v<Impl, OtherImpl>>::type
		>
		error_of(const error_of<OtherImpl>& err)
		{
			err_ = err.data();
		}

		error_of(const std::shared_ptr<Impl>& underlying)
		{
			err_ = underlying;
		}

		error_of(std::shared_ptr<Impl>& underlying)
		{
			err_ = underlying;
		}

		template <
			class T,
			class... Ts,
			class = std::enable_if<!std::is_same_v<Impl, error_interface>>::type
		>
		explicit error_of(T&& arg1, Ts&&... args)
		{
			err_ = std::make_shared<Impl>(std::forward<T>(arg1), std::forward<Ts>(args)...);
		}

		error_of()
		{
			if (!std::is_same_v<Impl, error_interface>)
				err_ = std::make_shared<Impl>();
		}

		~error_of() = default;

		operator bool() const noexcept
		{
			return err_.get() != nullptr;
		}

		std::string message() const
		{
			return err_->message();
		}

		std::shared_ptr<Impl> data() const
		{
			return err_;
		}

	private:
		std::shared_ptr<Impl> err_;
	};

	using error = error_of<error_interface>;
}

template <class A, class B>
bool operator==(const go::error_of<A> &a, const go::error_of<B>& b)
{
	return a.data().get() == b.data().get();
}

template <class A, class B>
bool operator!=(const go::error_of<A> &a, const go::error_of<B>& b)
{
	return !(a == b);
}

template <class Impl>
std::ostream& operator<<(std::ostream& os, const go::error_of<Impl>& err)
{
	return os << err.message();
}

