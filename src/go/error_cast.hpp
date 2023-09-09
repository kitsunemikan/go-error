#pragma once

#include <go/error.hpp>

#include <optional>

namespace go
{
	namespace detail
	{
		template <class To, class From>
		struct error_cast_impl
		{
			template <class T>
			struct always_false
			{
				static constexpr bool value = false;
			};

			static To cast(const error_of<From>&)
			{
				static_assert(always_false<To>::value, "unsupported error_cast");
			}
		};

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

		template <class ToImpl, class From>
		struct error_cast_impl<ToImpl*, From>
		{
			static ToImpl* cast(const error_of<From>& from)
			{
				return dynamic_cast<ToImpl*>(from.data().get());
			}
		};
	}

	/// I had an idea to return std::optional to let users determine whether
	/// cast failed or the underlying error was really empty. But on the other thought
	/// why would we need to cast an error anyway? Most probably from go::error to some
	/// concrete type, so we already know that that go::error is not empty.
	/// Additionally, if error was empty in the first place, the cast will return
	/// an empty error and no error logic will be triggered too.
	template <class To, class From>
	To error_cast(const error_of<From>& from)
	{
		return detail::error_cast_impl<To, From>::cast(from);
	}
}