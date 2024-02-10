#pragma once

#include <go/error.hpp>
#include <go/error_cast.hpp>

namespace go
{
    /// \cond TEMPLATE_DETAILS
	namespace detail
	{
        /*! \brief wrapping_impl is befriended by `go::error_of`, so that implementations
         * of `is_error` and `as_error` have access to the private details of
         * `go::error_of`.
         */
		struct wrapping_impl
		{
            /*! \brief `is_error` implementation that uses heap allocated stack and
             *  depth-first search.
             */
			template <class To>
			static auto is_error(error err, To& target) -> bool
			{
				if (!err && !target)
					return true;

				if (!err && target)
					return false;

				if (err && !target)
					return false;

				struct dfsStep
				{
					error err;
					int nextChildId;
				};
				static std::vector<dfsStep> errWalk(4); // 4 is arbitrary

				errWalk.resize(1);
				errWalk[0] = {err, 0};

				while (!errWalk.empty())
				{
					auto& errRef = errWalk.back();

					if (!errRef.err)
					{
						errWalk.pop_back();
						continue;
					}

					if (errRef.err == target)
						return true;

					if (errRef.err.is(target))
						return true;

					go::error unwrapped;

					// Skip checking unwrap if we already know that
					// unwrap_multiple is implemented
					if (errRef.nextChildId == 0)
						unwrapped = errRef.err.unwrap();

					if (unwrapped)
					{
						errRef = {unwrapped, 0};
						continue;
					}

					auto& unwrappedErrs = errRef.err.unwrap_multiple();
					if (unwrappedErrs.size() == 0)
					{
						errWalk.pop_back();
						continue;
					}

					if (errRef.nextChildId == unwrappedErrs.size())
					{
						errWalk.pop_back();
						continue;
					}

					errWalk.push_back({unwrappedErrs[errRef.nextChildId], 0});
					errRef.nextChildId++;
				}

				return false;
			}

            /*! \brief `as_error` implementation that uses heap allocated stack and
             *  depth-first search.
             */
			template <class To>
			static auto as_error(error err, To& target) -> bool
			{
				if (!err)
					return false;

				struct dfsStep
				{
					error err;
					int nextChildId;
				};
				static std::vector<dfsStep> errWalk(4); // 4 is arbitrary

				errWalk.resize(1);
				errWalk[0] = {err, 0};

				while (!errWalk.empty())
				{
					auto& errRef = errWalk.back();

					if (!errRef.err)
					{
						errWalk.pop_back();
						continue;
					}

					auto targetCandidate = error_cast<To>(errRef.err);
					if (targetCandidate)
					{
						target = targetCandidate;
						return true;
					}

					bool customAs = errRef.err.as(target);
					if (customAs)
						return true;

					go::error unwrapped;

					// Skip checking unwrap if we already know that
					// unwrap_multiple is implemented (nextChildId is not 0)
					if (errRef.nextChildId == 0)
						unwrapped = errRef.err.unwrap();

					if (unwrapped)
					{
						errRef = {unwrapped, 0};
						continue;
					}

					auto& unwrappedErrs = errRef.err.unwrap_multiple();
					if (unwrappedErrs.size() == 0)
					{
						errWalk.pop_back();
						continue;
					}

					if (errRef.nextChildId == unwrappedErrs.size())
					{
						errWalk.pop_back();
						continue;
					}

					errWalk.push_back({unwrappedErrs[errRef.nextChildId], 0});
					errRef.nextChildId++;
				}

				return false;
			}
		};
	} // namespace detail
    /// \endcond

    /*! \addtogroup wrapping
     * @{
     */

    /// `is_error` reports whether any error in err's tree matches target.
    /*!
     * The tree consists of err itself, followed by the errors obtained by repeatedly
     * calling its unwrap() or unwrap_multiple() method. When err wraps multiple errors,
     * `is_error` examines err followed by a depth-first traversal of its children.
     *
     * An error is considered to match a target if it is equal to that target or if
     * it implements `is_interface` for the target type such that is(target) returns true.
     *
     * An error type might implement `is_interface` so it can be treated as equivalent
     * to an existing error.
     *
     * An `is_interface` implementation should only shallowly compare err and the target
     * and not unwrap either. 
     */
	template <class Against>
	auto is_error(error err, const error_of<Against>& target) -> bool
	{
		return detail::wrapping_impl::is_error(err, target);
	}

	/// \brief Finds the first error in err's tree that matches target, and if one is found,
    /// sets target to that error value and returns true. Otherwise, it returns false.
    /*!
     * The tree consists of err itself, followed by the errors obtained by repeatedly
     * calling its unwrap() or unwrap_multiple() method. When err wraps multiple
     * errors, `as_error` examines err followed by a depth-first traversal of its children.
     *
     * An error matches target if the error's concrete value is `go::error_cast`-able to the value
     * pointed to by target, or if the error implements `go::as_interface` for that type.
     * In the latter case, the as method is responsible for setting target.
     *
     * Note that unlike go, `go::as_interface::as` method cannot fail. This is because
     * in go the as method accepts an unknown target type that may not be supported by the
     * error type. Thus, in such case it may return false. But in this library, types explicitly
     * declare what target types they support, and as_error automatically finds appropriate overloads.
     *
     * An error type might implement an `go::as_interface` so it can be treated as if it were a
     * different error type.
     *
     * `as_error` produces a compile error if target is const, not convertible to bool or
     * isn't supported by `go::error_cast`.
     */
	template <class To>
	auto as_error(error err, To& target) -> bool
	{
		static_assert(!std::is_const_v<To>, "as_error modifies target and expects it to be non-const");
		static_assert(std::is_convertible_v<To, bool>, "as_error expects target to be convertible to bool");
		static_assert(std::is_class_v<std::remove_pointer_t<std::remove_cv_t<To>>> || std::is_same_v<std::remove_cv_t<To>, void*>, "as_error expects target's type to be viable dynamic_cast target");

		return detail::wrapping_impl::as_error(err, target);
	}

    /*! @} */
}
