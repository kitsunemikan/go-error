#pragma once

#include <go/error.hpp>

#include <optional>

namespace go
{
	/// I had an idea to return std::optional to let users determine whether
	/// cast failed or the underlying error was really empty. But on the other thought
	/// why would we need to cast an error anyway? Most probably from go::error to some
	/// concrete type, so we already know that that go::error is not empty.
	/// Additionally, if error was empty in the first place, the cast will return
	/// an empty error and no error logic will be triggered too.
	template <class To, class From>
	To error_cast(const error_of<From>& from)
	{
		std::shared_ptr<To::impl_type> toImpl = std::dynamic_pointer_cast<To::impl_type>(from.data());

		if (toImpl.get() == nullptr)
		{
			return {};
		}

		return To(toImpl);
	}
}