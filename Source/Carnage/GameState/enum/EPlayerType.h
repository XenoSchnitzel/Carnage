#pragma once

#include "EPlayerType.generated.h"

UENUM(BlueprintType)
enum class EPlayerType : uint8
{
    Human       UMETA(DisplayName = "Human"),
    Computer    UMETA(DisplayName = "Computer")
};
