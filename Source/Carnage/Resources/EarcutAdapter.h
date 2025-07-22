#pragma once

#include "CoreMinimal.h"
#include <tuple>

// Needed to make FVector2D work with mapbox::earcut
namespace std
{
    template<>
    struct tuple_size<FVector2D> : std::integral_constant<size_t, 2> {};

    template<>
    struct tuple_element<0, FVector2D> { using type = double; };

    template<>
    struct tuple_element<1, FVector2D> { using type = double; };

    template<>
    constexpr double& get<0>(FVector2D& v) noexcept { return v.X; }

    template<>
    constexpr double& get<1>(FVector2D& v) noexcept { return v.Y; }

    template<>
    constexpr const double& get<0>(const FVector2D& v) noexcept { return v.X; }

    template<>
    constexpr const double& get<1>(const FVector2D& v) noexcept { return v.Y; }
}
