// ===== File: ECarnageUnitType.h =====
#pragma once
#include "CoreMinimal.h"
#include "ECarnageUnitType.generated.h"

/**
 * Enum der Einheitentypen für das Carnage‑RTS. Bewusst "ECarnageUnitType" genannt, um Namenkollisionen
 * mit möglichen Engine‑internen "EUnitType"‑Deklarationen zu vermeiden.
 */
UENUM(BlueprintType)
enum class ECarnageUnitType : uint8
{
    Worker      UMETA(DisplayName = "Worker"),
    Soldier     UMETA(DisplayName = "Soldier"),
    // … weitere Typen
};

