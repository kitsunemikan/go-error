#include <go/error.hpp>

namespace go
{

	error error_interface::unwrap() const
	{
		return error_of<error_interface>();
	}

	const std::vector<error>& error_interface::unwrap_multiple() const
	{
		return {};
	}

	bool error_interface::is(error other) const
	{
		return this == other.data().get();
	}
}