#include <go/go_error.hpp>

#include <chrono>
#include <string>
#include <sstream>
#include <iostream>

struct error_my_data : public go::error_interface
{
	time_t when;
	std::string what;

	std::string message() const override
	{
		std::stringstream ss;
		ss << "When: " << std::ctime(&when) << "What: " << what;
		return ss.str();
	}
};

using error_my = go::error_of<error_my_data>;

go::error oops()
{
    auto now = std::chrono::system_clock::now();

    auto err = go::make_error<error_my>();
    err->when = std::chrono::system_clock::to_time_t(now);
    err->what = "the file system has gone away";

    return err;
}

int main()
{
    if (auto err = oops())
    {
        std::cout << err << '\n';
    }
    
    return 0;
}
