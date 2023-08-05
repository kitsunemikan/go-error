#pragma once

#include <go/error.hpp>

#include <system_error>
#include <sstream>

namespace go
{
	namespace impl
	{
		struct error_code : public error_interface
		{
			explicit error_code(const std::error_code& ec) :
				ec_(ec)
			{}

			std::error_code code() const
			{
				return ec_;
			}

			int value() const
			{
				return ec_.value();
			}

			std::string message() const override
			{
				std::stringstream ss;
				ss << ec_.message() << " (error code: " << ec_.value() << ")";
				return ss.str();
			}

		private:
			std::error_code ec_;
		};
	}

	using error_code = error_of<impl::error_code>;
}