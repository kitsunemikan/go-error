#pragma once

#include <type_traits>
#include <string>
#include <memory>
#include <vector>

namespace go
{
	namespace detail
	{
		struct wrapping_impl;
	}

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

		virtual ~error_interface() noexcept = default;
	};


	// is_interface contains virtual 'is' methods for specified types.
	template <class... Targets>
	struct is_interface;

	template <class Target, class... Rest>
	struct is_interface<Target, Rest...> : public is_interface<Target>, public is_interface<Rest...> {};

	// Enables custom logic in `is_error` for errors
	// `bool is(Target&)` method is expected to immutable inspect target
	// and return a boolean value as per error's custom logic
	template <class Target>
	struct is_interface<Target>
	{
		virtual bool is(const Target&) const = 0;
	};


	// as_interface contains virtual 'as' methods for specified types.
	template <class... Targets>
	struct as_interface;

	template <class Target, class... Rest>
	struct as_interface<Target, Rest...> : public as_interface<Target>, public as_interface<Rest...> {};

	// Enables custom logic in `as_error` for errors
	// `as(Target&)` method is expected to modify Target value as per
	// error's custom logic. It's assumed the method always succeeds
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
			if (!err_)
				return error();

			return err_->unwrap();
		}

		const std::vector<error>& unwrap_multiple() const
		{
			static std::vector<error> empty{};

			if (!err_)
				return empty;

			return err_->unwrap_multiple();
		}

	private:
		std::shared_ptr<Impl> err_;

		// TODO: Target&& -> class = has const and Target is ref, otherwise non-const rvalue is ok
		template <class Target>
		bool is(const Target& other) const
		{
			// Check that our error Impl type implemented is_interface for
			// custom `is` behavior
			auto ptr = dynamic_cast<is_interface<Target>*>(err_.get());
			if (!ptr)
				return false;

			return ptr->is(other);
		}

		// Expects an lvalue reference
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

		// We want users to use free `is_error` and `as_error` functions
		// instead of is and as methods that lack safety checks present
		// in these free functions
		friend class detail::wrapping_impl;
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

