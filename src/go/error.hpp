#pragma once

#include <type_traits>
#include <string>
#include <memory>
#include <vector>

namespace go
{
	template <class Impl>
	struct error_of;

	struct error_interface;

	using error = error_of<error_interface>;

	struct error_interface
	{
		virtual std::string message() const = 0;

		virtual error unwrap() const;

		/// For the is and as functions to use `unwrap_multi`, `unwrap` should return empty error
		virtual const std::vector<error>& unwrap_multiple() const;

		virtual bool is(error other) const;

		virtual ~error_interface() noexcept = default;
	};

	template <class... Targets>
	struct as_interface;

	template <class Target, class... Rest>
	struct as_interface<Target, Rest...> : public as_interface<Target>, public as_interface<Rest...> {};

	template <class Target>
	struct as_interface<Target>
	{
		virtual void as(Target&) const = 0;
	};

	// Any concrete type can also be null, that's really important
	// because concrete type and generic type correlate to an interface
	// and an actual pointer: *MyError and error.
	template <class Impl>
	struct error_of
	{
		static_assert(std::is_base_of_v<error_interface, Impl>,
			"error implementation should inherit from error_interface");

		using impl_type = Impl;

		error_of() = default;

		~error_of() = default;

		template<
			class OtherImpl,
			class = std::enable_if_t<std::is_base_of_v<Impl, OtherImpl>>
		>
		error_of(const error_of<OtherImpl>& err)
		{
			err_ = err.data();
		}

		template<
			class OtherImpl,
			class = std::enable_if_t<std::is_base_of_v<Impl, OtherImpl>>
		>
		error_of(error_of<OtherImpl>&& err)
		{
			err_ = err.data();
			err = {};
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
			class = std::enable_if_t<!std::is_same_v<Impl, error_interface>>
		>
		explicit error_of(T&& arg1, Ts&&... args)
		{
			err_ = std::make_shared<Impl>(std::forward<T>(arg1), std::forward<Ts>(args)...);
		}

		operator bool() const noexcept
		{
			return err_.get() != nullptr;
		}

		std::string message() const
		{
			if (!err_)
				return "<nil>";

			return err_->message();
		}

		std::shared_ptr<Impl> data() const
		{
			return err_;
		}

		error unwrap() const
		{
			return err_->unwrap();
		}

		const std::vector<error>& unwrap_multiple() const
		{
			return err_->unwrap_multiple();
		}

		bool is(error other) const
		{
			return err_->is(other);
		}

		template <class Target>
		bool as(Target& target) const
		{
			// Check that our error Impl type implemented as_interface for
			// custom `as` behavior
			auto ptr = dynamic_cast<as_interface<Target>*>(err_.get());
			if (!ptr)
				return false;

			ptr->as(target);
			return true;
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

/*
template <class Impl, class Ptr>
bool operator==(const go::error_of<Impl> &a, Ptr*& b)
{
	return a.data().get() == b;
}

template <class Impl, class Ptr>
bool operator!=(const go::error_of<Impl> &a, Ptr*& b)
{
	return !(a == b);
}

template <class Impl, class Ptr>
bool operator==(Ptr*& a, const go::error_of<Impl> &b)
{
	return a == b.data().get();
}

template <class Impl, class Ptr>
bool operator!=(Ptr*& a, const go::error_of<Impl> &b)
{
	return !(a == b);
}
*/

template <class Impl>
std::ostream& operator<<(std::ostream& os, const go::error_of<Impl>& err)
{
	return os << err.message();
}

