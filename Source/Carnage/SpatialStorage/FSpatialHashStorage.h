#pragma once

#include "CoreMinimal.h"
#include "IUnitStorageStrategy.h"
#include "../GameState/enum/EHostility.h"
#include "GameFramework/Actor.h"
#include <unordered_map>
#include <unordered_set>

class FSpatialHashStorage : public IUnitStorageStrategy
{
public:
    explicit FSpatialHashStorage(float InCellSize = 1000.0f);
    virtual ~FSpatialHashStorage() override = default;

    virtual void AddUnit(AActor* Unit, EHostility Team) override;
    virtual void RemoveUnit(AActor* Unit) override;
    virtual AActor* FindNearestUnit(const FVector2D& Position, EHostility EnemyTeam) const override;
    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) override;

private:
    float CellSize;

    struct FInt2DKey
    {
        int32 X;
        int32 Y;

        FInt2DKey(int32 InX = 0, int32 InY = 0) : X(InX), Y(InY) {}

        bool operator==(const FInt2DKey& Other) const { return X == Other.X && Y == Other.Y; }

        friend uint32 GetTypeHash(const FInt2DKey& Key)
        {
            return HashCombine(::GetTypeHash(Key.X), ::GetTypeHash(Key.Y));
        }
    };

    struct FInt2DKeyHasher
    {
        std::size_t operator()(const FInt2DKey& k) const
        {
            return std::hash<int32>()(k.X) ^ (std::hash<int32>()(k.Y) << 1);
        }
    };

    // Separate Hash Grids f�r Friendly (0) und Enemy (1)
    TMap<FInt2DKey, TSet<AActor*>> TeamGrids[2];

    // Tracking Maps
    TMap<AActor*, FInt2DKey> UnitToCellMap;
    TMap<AActor*, EHostility> UnitToTeamMap;
    TMap<AActor*, FVector2D> UnitToPositionMap;

    FInt2DKey GetCellForPosition(const FVector2D& Position) const;
};


//#pragma once
//
//#include "CoreMinimal.h"
//#include "IUnitStorageStrategy.h"
//#include "EHostility.h"
//
//class FSpatialHashStorage : public IUnitStorageStrategy
//{
//public:
//    FSpatialHashStorage(float InCellSize = 1000.0f);
//    virtual ~FSpatialHashStorage() = default;
//
//    virtual void AddUnit(AActor* Unit, EHostility Team) override;
//    virtual void RemoveUnit(AActor* Unit) override;
//    virtual AActor* FindNearestUnit(const FVector2D& Position, EHostility EnemyTeam) const override;
//    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) override;
//
//private:
//    float CellSize;
//
//    struct FInt2DKey
//    {
//        int32 X;
//        int32 Y;
//
//        FInt2DKey(int32 InX = 0, int32 InY = 0) : X(InX), Y(InY) {}
//
//        bool operator==(const FInt2DKey& Other) const
//        {
//            return X == Other.X && Y == Other.Y;
//        }
//
//        friend uint32 GetTypeHash(const FInt2DKey& Key)
//        {
//            return HashCombine(::GetTypeHash(Key.X), ::GetTypeHash(Key.Y));
//        }
//    };
//
//    // Zwei Grids: Index 0 = Friendly, 1 = Enemy
//    TMap<FInt2DKey, TSet<AActor*>> TeamGrids[2];
//
//    TMap<AActor*, FInt2DKey> UnitToCellMap;
//    TMap<AActor*, EHostility> UnitToTeamMap;
//    TMap<AActor*, FVector2D> UnitToPositionMap;
//
//    FInt2DKey GetCellForPosition(const FVector2D& Position) const;
//
//    void UpdateUnit(AActor* Unit, const FVector2D& NewPosition);
//};

