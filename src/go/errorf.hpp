#pragma once

#include <go/error.hpp>
#include <go/error_string.hpp>

#include <sstream>

namespace go
{
	template <class... Ts>
	go::error errorf(Ts&&... args)
	{
		std::stringstream stream;
		(stream << ... << std::forward<Ts>(args));

		return go::error_string(stream.str());
	}
}