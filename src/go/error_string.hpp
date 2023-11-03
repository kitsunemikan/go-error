#pragma once

#include <go/error.hpp>

namespace go
{
    struct error_string_data : public error_interface
    {
        error_string_data(std::string msg) : msg(std::move(msg)) {}

        std::string message() const override {
            return msg;
        }

    private:
        std::string msg;
    };

	using error_string = go::error_of<error_string_data>;
}
