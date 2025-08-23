#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Melee    UMETA(DisplayName = "Melee"),
    Distance UMETA(DisplayName = "Distance")
};
