#pragma once

#include <go/error.hpp>

namespace go
{
    /*! \addtogroup predefined Predefined errors
     * @{
     */

    /// Error data for `go::error_string`.
    struct error_string_data : public error_interface
    {
        /// Initialize with a predefined message.
        error_string_data(std::string msg) : msg(std::move(msg)) {}

        /// Returns the string used to initialize data as-is.
        auto message() const override -> std::string {
            return msg;
        }

    private:
        std::string msg;
    };

    /// Simple error type that encapsulates a single predefined string.
	using error_string = go::error_of<error_string_data>;

    /*! @} */
}
