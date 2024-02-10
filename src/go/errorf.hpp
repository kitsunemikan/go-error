#pragma once

#include <go/error.hpp>
#include <go/error_string.hpp>

#include <sstream>

namespace go
{
    /*! \addtogroup core
     * @{
     */

    /// A `go::error_string`-like error formater that allows to wrap errors.
    /*!
     * The arguments passed to the functions are forwarded to a `std::stringstream`
     * and the resulting message is used to initialized a user-unknown error type.
     *
     * Additionally, any errors passed as arguments, while being formatted as strings,
     * are also gonna be wrapped by the resulting error. (TODO #32)
     */
	template <class... Ts>
	go::error errorf(Ts&&... args)
	{
		std::stringstream stream;
		(stream << ... << std::forward<Ts>(args));

		return go::make_error<go::error_string>(stream.str());
	}

    /*! @} */
}
