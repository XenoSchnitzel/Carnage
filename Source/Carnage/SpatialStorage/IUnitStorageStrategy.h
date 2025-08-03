#pragma once

#include "CoreMinimal.h"
#include "../GameState/enum/EFaction.h"
#include "GameFramework/Actor.h"

class IUnitStorageStrategy
{
public:
    virtual ~IUnitStorageStrategy() = default;

    virtual void AddUnit(AActor* Unit, EFaction myTeam) = 0;
    virtual void RemoveUnit(AActor* Unit) = 0;
    virtual AActor* FindNearestUnit(const FVector2D& Position, EFaction myFaction) const = 0;

    // Neu für Positions-Update
    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) = 0;
};


