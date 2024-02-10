#pragma once

#include <go/error.hpp>
#include <go/detail/meta_helpers.hpp>

#include <optional>

namespace go
{
    /// \cond TEMPLATE_DETAILS
	namespace detail
	{
        /// Fallback primary template that causes compilation error
		template <class To, class From>
		struct error_cast_impl
		{
			static To cast(const error_of<From>&)
			{
				static_assert(always_false<To>::value, "unsupported error_cast");
			}
		};

        /// Implementation for error to error type conversion
		template <class ToImpl, class From>
		struct error_cast_impl<error_of<ToImpl>, From>
		{
			static error_of<ToImpl> cast(const error_of<From>& from)
			{
				std::shared_ptr<ToImpl> toImpl = std::dynamic_pointer_cast<ToImpl>(from.data());

				if (toImpl.get() == nullptr)
				{
					return {};
				}

				return {toImpl};
			}
		};

        /// Implementation for a conversion to error data or compatible interface type
		template <class ToImpl, class From>
		struct error_cast_impl<ToImpl*, From>
		{
			static ToImpl* cast(const error_of<From>& from)
			{
				return dynamic_cast<ToImpl*>(from.data().get());
			}
		};
	} // namespace detail
    /// \endcond

    /*! \addtogroup core
     * @{
     */

    /*! \name Error casting */
    ///@{

    /*!
     *  \brief Cast an error to the specified type.
     *
     *  This function can perform upcast/downcast-like casts between `go::error_of`
     *  types, as well as to the underlying error data types of a particular error.
     *
     *  If casting is performed to an unsupported type a compilation error is produced.
     *  Supported types are instantiations of `go::error_of` or a pointer.
     *
     *  If cast failed, like trying to convert one error type to an unrelated error type,
     *  an empty error is returned.
     *
     *  If the input error is empty, an empty error is returned too.
     *
     *  The returned error will have the same instance of underlying error data.
     *
     *  Examples:
     *  ```
     *  struct some_interface;
     *  struct error_base_data;
     *  struct error_derived_data : public error_base_data, public some_interface;
     *
     *  using error_base = go::error_of<error_base_data>;
     *  using error_derived = go::error_of<error_derived_data>;
     *
     *  auto errBase = go::make_error<error_base>();
     *  auto errDeri = go::make_error<error_derived>();
     *
     *  assert(go::error_cast<error_base>(errDeri) == errBase);
     *  assert(go::error_cast<error_base::impl_type>(errDeri) == errBase.data().get());
     *  assert(go::error_cast<some_interface*>(errDeri) == (some_interface*)errBase.data().get());
     *  ```
     */
	template <class To, class From>
	To error_cast(const error_of<From>& from)
	{
        // I had an idea to return std::optional to let users determine whether
        // cast failed or the underlying error was really empty. But on the other thought
        // why would we need to cast an error anyway? Most probably from go::error to some
        // concrete type, so we already know that that go::error is not empty.
        // Additionally, if error was empty in the first place, the cast will return
        // an empty error and no error logic will be triggered too.

		return detail::error_cast_impl<To, From>::cast(from);
	}

    ///@}
    /*! @} */
}
