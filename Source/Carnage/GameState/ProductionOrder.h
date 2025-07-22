#pragma once
#include "CoreMinimal.h"
#include "ECarnageUnitType.h"
#include "ProductionOrder.generated.h"

USTRUCT(BlueprintType)
struct FProductionOrder
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ECarnageUnitType UnitType;

    UPROPERTY(BlueprintReadWrite)
    float BuildTime;

    UPROPERTY(BlueprintReadWrite)
    int32 UnitId;

    float ElapsedTime = 0.0f;
};