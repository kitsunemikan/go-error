#pragma once

#include <string>

namespace go
{

	struct error
	{
		explicit error(const char* str) :
			msg_(str)
		{}

		std::string message() const
		{
			return msg_;
		}

	private:
		std::string msg_;
	};

}