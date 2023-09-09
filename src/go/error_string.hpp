#pragma once

#include <go/error.hpp>

namespace go
{
	namespace impl
	{
		struct error_string : public error_interface
		{
			error_string(std::string msg) : msg(std::move(msg)) {}

			std::string message() const override {
				return msg;
			}

		private:
			std::string msg;
		};
	}

	using error_string = go::error_of<impl::error_string>;
}
