#pragma once

#include <go/error.hpp>
#include <go/error_cast.hpp>

namespace go
{
	namespace detail
	{

		struct wrapping_impl
		{
			template <class To>
			static bool is_error(error err, To& target)
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

			template <class To>
			static bool as_error(error err, To& target)
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
	}

    /*! \addtogroup wrapping Wrapping
     * @{
     */

    /*! is impl
     */
	template <class Against>
	bool is_error(error err, const error_of<Against>& target)
	{
		return detail::wrapping_impl::is_error(err, target);
	}

	/*! To must be convertible to bool
	 *  To must be supported by error_cast
     */
	template <class To>
	bool as_error(error err, To& target)
	{
		static_assert(!std::is_const_v<To>, "as_error modifies target and expects it to be non-const");
		static_assert(std::is_convertible_v<To, bool>, "as_error expects target to be convertible to bool");
		static_assert(std::is_class_v<std::remove_pointer_t<std::remove_cv_t<To>>> || std::is_same_v<std::remove_cv_t<To>, void*>, "as_error expects target's type to be viable dynamic_cast target");

		return detail::wrapping_impl::as_error(err, target);
	}

    /*! @} */
}
