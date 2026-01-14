#pragma once

#include "EUIHitType.generated.h"

UENUM(BlueprintType)
enum class EUIHitType : uint8
{
    None    UMETA(DisplayName = "None"),
    Minimap UMETA(DisplayName = "Minimap")
};