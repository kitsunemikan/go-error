#pragma once

#include <go/error.hpp>

namespace go
{
	namespace impl
	{
		struct error_string : public error_interface
		{
			error_string() = default;

			error_string(const std::string& msg) :
				msg(msg)
			{}

			std::string message() const override {
				return msg;
			}

		private:
			std::string msg;
		};
	}

	using error_string = go::error_of<impl::error_string>;
}
