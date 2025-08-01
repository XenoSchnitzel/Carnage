#pragma once

#include "CoreMinimal.h"
#include "../GameState/enum/EHostility.h"
#include "GameFramework/Actor.h"

class IUnitStorageStrategy
{
public:
    virtual ~IUnitStorageStrategy() = default;

    virtual void AddUnit(AActor* Unit, EHostility Team) = 0;
    virtual void RemoveUnit(AActor* Unit) = 0;
    virtual AActor* FindNearestUnit(const FVector2D& Position, EHostility EnemyTeam) const = 0;

    // Neu für Positions-Update
    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) = 0;
};


