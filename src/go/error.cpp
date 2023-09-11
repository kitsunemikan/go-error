#include <go/error.hpp>

namespace go
{

	error error_interface::unwrap() const
	{
		return {};
	}

	const std::vector<error>& error_interface::unwrap_multiple() const
	{
        static std::vector<error> dummy;
		return dummy;
	}

}
