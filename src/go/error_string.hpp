#pragma once

#include <go/error.hpp>

namespace go
{
	struct error_string : public errorlike
	{
		explicit error_string(const std::string& msg)
		{
			err_ = std::make_shared<impl>(msg);
		}

		std::string message() const override
		{
			return err_->message();
		}

		std::shared_ptr<error_interface> bare_error() const override
		{
			return err_;
		}

	private:
		struct impl : public error_interface
		{
			std::string msg;

			impl(const std::string& msg) :
				msg(msg)
			{}

			std::string message() const override {
				return msg;
			}
		};

		std::shared_ptr<impl> err_;
	};
}
