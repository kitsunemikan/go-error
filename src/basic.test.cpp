#include <go/error.hpp>

#include <boost/ut.hpp>
using namespace boost::ut;

int main()
{
    "test"_test = [] {
        expect(1 == 0) << "got" << false << "but want" << true;
    };

    return 0;
}
