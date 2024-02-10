#pragma once

/// \cond TEMPLATE_DETAILS
namespace go::detail
{

    /// \brief always_false can be used in static asserts to uncoditionally
    /// fail compilation, but only at instantiation stage
    template <class T>
    struct always_false
    {
        static constexpr bool value = false;
    };

}
/// \endcond
