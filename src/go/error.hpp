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

    /*! \addtogroup core Core
     * @{
     */

    /// Core error interface to be implemented by user-defined errors.
    /*!
     * The interface emulates interface for go's errors. The Error() method is
     * represented by the `message()` pure virtual method, which is named to be
     * consistent with the message() method of std::error_code.
     *
     * As in go, the user can additionally implement either unwrap of unwrap_multiple
     * methods to provide wrapping functionality. This can be used to provide nested
     * context to the error and allow user code to decide its execution based on the
     * inner error types. The `go::is_error` and `go::as_error` functions can be used
     * to introspect such wrapped errors.
     *
     * To implement a custom error type, the user needs to define a structure, referred
     * to as error data, that implements this interface, and then create a type alias to
     * the error_of<T> of the error data. By convention, error data is suffixed
     * with _data, while the alias is written without it.
     *
     * It is possible to use `go::error_cast` to extract error data from an
     * error_of<T> error and access error data's custom methods in case user written any.
     */
	struct error_interface
	{
        /// Returns the error message string.
		virtual auto message() const -> std::string = 0;

        /// Returns a single wrapped error if any. Default implementation returns empty error.
		virtual auto unwrap() const -> error;

		/// Returns an array of wrapped errors.
        /*! For the `go::is_error` and `go::as_error` functions to use `unwrap_multiple`,
         * `unwrap` should return empty error.
         */
		virtual auto unwrap_multiple() const -> std::vector<error> const&;

		virtual ~error_interface() noexcept = default;
	};

    /*! @} */

    /*! \addtogroup wrapping Wrapping
     * @{
     */

	/// Helper for declaring `go::is_error` customizations for multiple types at once.
	template <class... Targets>
	struct is_interface;

    /*! \cond TEMPLATE_DETAILS */

	template <class Target, class... Rest>
	struct is_interface<Target, Rest...> : public is_interface<Target>, public is_interface<Rest...> {};

    /*! \endcond */

    /// Enables custom logic in `go::is_error` for errors.
	template <class Target>
	struct is_interface<Target>
	{
        /// Check that the target error is the current one as per custom logic.
		virtual auto is(Target const&) const -> bool = 0;
	};


	/// Helper for declaring `go::as_error` customizations for multiple types at once.
	template <class... Targets>
	struct as_interface;

    /*! \cond TEMPLATE_DETAILS */

	template <class Target, class... Rest>
	struct as_interface<Target, Rest...> : public as_interface<Target>, public as_interface<Rest...> {};

    /*! \endcond */

    /// Enables custom logic in `go::as_error` for errors.
	template <class Target>
	struct as_interface<Target>
	{
        /// Convert current error type to the target type and overwrite target error.
        /*!
         * Refer to `go::as_error` documentation for why it returns void instead of bool
         * as in go.
         */
		virtual auto as(Target&) const -> void = 0;
	};

    /*! @} */

    /*! \addtogroup core
     * @{
     */

	/// \brief Core library class that implements error semantics for arbitrary error
    /// interface implementations.
    /*! 
     * This class is analogous to the `error` interface in go. This is in comparison
     * to the error data objects that represent actual implementations, like `MyError*`
     * and that are retrieved by type assertions, like `myErr := err.(MyError*)` in go.
     *
     * The error type can be either empty or contain an instance of error data. As
     * in go, the error type may be non-empty while underlying data is null. This is
     * one of the quirks of the go errors that the users need to be aware of
     *
     * ```
     * auto quirkExample() -> error {
     *     // Zero-initialized
     *     auto err = std::shared_ptr<MyError>();
     *
     *     if (...)
     *     {
     *         // Updated error with value if something goes wrong
     *         err = std::make_shared<MyError>();
     *     }
     *
     *     // In case of no error a null MyError*
     *     // will be converted to an non-empty error
     *     return err
     * }
     * ```
     *
     * error_of has the same overhead as std::shared_ptr.
     *
     * Two errors are equal only if their error data pointer values are equal. So even
     * if error data is of the same type and the same content, but of different
     * instances, the errors are considered to be different.
     *
     * Errors are implicitly upcasted when needed. It is expected that users
     * use concrete error classes only when initializing errors or extracting
     * errors from generic error class.
     */
	template <class Impl>
	struct error_of
	{
		static_assert(std::is_base_of_v<error_interface, Impl>,
			"error implementation should inherit from error_interface");

        /// Typedef for the template error data type
		using impl_type = Impl;

		error_of() = default;

		~error_of() = default;

        /// Copy constructor.
		template<
			class OtherImpl,
			class = std::enable_if_t<std::is_base_of_v<Impl, OtherImpl>>
		>
		error_of(error_of<OtherImpl> const& err)
		{
			err_ = err.data();
		}

        /// Move constructor.
		template<
			class OtherImpl,
			class = std::enable_if_t<std::is_base_of_v<Impl, OtherImpl>>
		>
		error_of(error_of<OtherImpl>&& err)
		{
			err_ = err.data();
			err = {};
		}

        /// Construct via existing error data instance.
		error_of(std::shared_ptr<Impl> underlying)
		{
			err_ = std::move(underlying);
		}

        /// False if error is empty, false otherwise. `data()` may still be null.
		operator bool() const noexcept
		{
			return err_.get() != nullptr;
		}

        /// \brief Returns error message as implemented by error data or
        /// a nil-indicating string if error data is null.
		auto message() const -> std::string
		{
			if (!err_)
				return "<nil>";

			return err_->message();
		}

        /// Returns error data instance.
		auto data() const -> std::shared_ptr<Impl>
		{
			return err_;
		}

        /// \brief Returns a wrapped error or an empty error
        /// if error data doesn't implement unwrap.
		auto unwrap() const -> error
		{
			if (!err_)
				return error();

			return err_->unwrap();
		}

        /// \brief Returns an array of wrapped errors or an empty array
        /// if error data doesn't implement unwrap_multiple.
		auto unwrap_multiple() const -> std::vector<error> const&
		{
			static std::vector<error> empty{};

			if (!err_)
				return empty;

			return err_->unwrap_multiple();
		}

        /// Operator overload to the error's data.
        auto operator->() const -> Impl*
        {
            return err_.get();
        }

	private:
		std::shared_ptr<Impl> err_;

		// TODO: Target&& -> class = has const and Target is ref, otherwise non-const rvalue is ok
		template <class Target>
		auto is(Target const& other) const -> bool
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
		auto as(Target& target) const -> bool
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

    /// Main error type. Expected to be returned from functions and used most of the time.
    /*!
     * It is analogous to go's `error` type. Users can use `go::error_cast`, `go::is_error`
     * and `go::as_error` to introspect errors.
     */
	using error = error_of<error_interface>;


    namespace detail
    {

        template <class ErrorType>
        struct make_error_impl
        {
            template <class... Args>
            static auto make(Args&&... args) -> ErrorType
            {
                static_assert(always_false<ErrorType>, "ErrorType should a valid go::error_of<T>");
            }
        };

        template <class Impl>
        struct make_error_impl<error_of<Impl>>
        {
            template <class... Args>
            static auto make(Args&&... args) -> error_of<Impl>
            {
                auto impl = std::make_shared<Impl>(std::forward<Args>(args)...);
                return error_of<Impl>(std::move(impl));
            }
        };
    }

    /// Construct via error data constructor.
    template <
        class ErrorType,
        class... Args
    >
    auto make_error(Args&&... args) -> ErrorType
    {
        return detail::make_error_impl<ErrorType>::make(std::forward<Args>(args)...);
    }

    /*! @} */
}

/*! \addtogroup core
 * @{
 */

/// Errors are equal only if the error data pointer addresses are equal.
template <class A, class B>
auto operator==(go::error_of<A> const& a, go::error_of<B> const& b) -> bool
{
	return a.data().get() == b.data().get();
}

/// Refer to `operator==`
template <class A, class B>
auto operator!=(go::error_of<A> const& a, go::error_of<B> const& b) -> bool
{
	return !(a == b);
}

/*! \name Helper operator overloads */
///@{

/// Overload for std::ostream that outputs error.message() to the stream.
template <class Impl>
auto operator<<(std::ostream& os, go::error_of<Impl> const& err) -> std::ostream&
{
	return os << err.message();
}

///@}
/*! @} */
