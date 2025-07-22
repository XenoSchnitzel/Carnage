#pragma once

#include "ETeam.generated.h"

UENUM(BlueprintType)
enum class ETeam : uint8
{
    Friendly UMETA(DisplayName = "Friendly"),
    Enemy    UMETA(DisplayName = "Enemy")
};
