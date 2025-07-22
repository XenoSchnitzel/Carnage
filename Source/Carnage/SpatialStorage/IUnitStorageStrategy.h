
//#pragma once
//
//#include "CoreMinimal.h"
//#include "ETeam.h"
//
//class IUnitStorageStrategy
//{
//public:
//    virtual ~IUnitStorageStrategy() {}
//
//    virtual void AddUnit(AActor* Unit, ETeam Team) = 0;
//    virtual void RemoveUnit(AActor* Unit) = 0;
//    virtual AActor* FindNearestUnit(const FVector2D& Position, ETeam EnemyTeam) const = 0;
//    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) const = 0;
//};


#pragma once

#include "CoreMinimal.h"
#include "ETeam.h"
#include "GameFramework/Actor.h"

class IUnitStorageStrategy
{
public:
    virtual ~IUnitStorageStrategy() = default;

    virtual void AddUnit(AActor* Unit, ETeam Team) = 0;
    virtual void RemoveUnit(AActor* Unit) = 0;
    virtual AActor* FindNearestUnit(const FVector2D& Position, ETeam EnemyTeam) const = 0;

    // Neu für Positions-Update
    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) = 0;
};


