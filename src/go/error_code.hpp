#pragma once

#include <go/error.hpp>

#include <system_error>
#include <sstream>

namespace go
{
    /*! \addtogroup predefined Predefined errors
     * @{
     */

    /// Error data for `go::error_code`
    /*!
     * Adapts `std::error_code` message and value methods.
     *
     * Additionally, helper methods provide a more convenient access
     * to `std::error_code`'s members.
     */
    struct error_code_data : public error_interface
    {
        /// Initialized using existing error code.
        explicit error_code_data(std::error_code ec) :
            ec_(std::move(ec))
        {}

        /// Returns underlying `std::error_code`.
        auto code() const -> std::error_code
        {
            return ec_;
        }

        /// Returns underlying `std::error_code`'s value.
        auto value() const -> int
        {
            return ec_.value();
        }

        /// \brief The error's message consists of `std::error_code` accompanied with
        /// an error code value in decimal.
        auto message() const override -> std::string
        {
            std::stringstream ss;
            ss << ec_.message() << " (error code: " << ec_.value() << ")";
            return ss.str();
        }

    private:
        std::error_code ec_;
    };

    /// error_code represents a wrapper around `std::error_code`.
    /*!
     * Refer to `go::error_code_data` for behavior details.
     */
	using error_code = error_of<error_code_data>;

    /*! @} */
}
