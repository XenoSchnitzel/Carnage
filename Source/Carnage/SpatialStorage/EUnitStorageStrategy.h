#pragma once

#include "EUnitStorageStrategy.generated.h"

UENUM(BlueprintType)
enum class EUnitStorageStrategy : uint8
{
    SpatialHashing    UMETA(DisplayName = "Spatial Hashing"),
    Quadtree          UMETA(DisplayName = "Quadtree")
};