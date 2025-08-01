#pragma once

#include "EHostility.generated.h"

UENUM(BlueprintType)
enum class EHostility : uint8
{
    Friendly UMETA(DisplayName = "Friendly"),
    Enemy    UMETA(DisplayName = "Enemy")
};
