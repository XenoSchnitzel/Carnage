#pragma once

#include "CoreMinimal.h"
#include "IUnitStorageStrategy.h"
#include "ETeam.h"
#include "GameFramework/Actor.h"

class FQuadTreeStorage : public IUnitStorageStrategy
{
public:
    FQuadTreeStorage(const FBox2D& InBounds, int32 InMaxDepth = 5, int32 InMaxObjectsPerNode = 10);
    virtual ~FQuadTreeStorage() override = default;

    virtual void AddUnit(AActor* Unit, ETeam Team) override;
    virtual void RemoveUnit(AActor* Unit) override;
    virtual AActor* FindNearestUnit(const FVector2D& Position, ETeam EnemyTeam) const override;
    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) override;

private:
    struct FQuadTreeNode
    {
        FBox2D Bounds;
        int32 Depth;
        TArray<AActor*> Objects;
        TUniquePtr<FQuadTreeNode> Children[4];
        bool bIsLeaf;

        FQuadTreeNode(const FBox2D& InBounds, int32 InDepth);

        void Subdivide();
        bool Insert(AActor* Unit, const FVector2D& Pos, int32 MaxDepth, int32 MaxObjectsPerNode);
        bool Remove(AActor* Unit, const FVector2D& Pos);
        void Query(const FVector2D& Position, float& ClosestDistSq, AActor*& ClosestUnit) const;
    };

    TUniquePtr<FQuadTreeNode> RootNodes[2];

    int32 MaxDepth;
    int32 MaxObjectsPerNode;

    TMap<AActor*, ETeam> UnitToTeamMap;
    TMap<AActor*, FVector2D> UnitToPositionMap;

    void InsertUnit(AActor* Unit, ETeam Team);
    void RemoveUnitFromTree(AActor* Unit, ETeam Team);
    AActor* FindNearestInTeam(const FVector2D& Position, ETeam Team) const;
};

//#pragma once
//
//#include "CoreMinimal.h"
//#include "IUnitStorageStrategy.h"
//#include "ETeam.h"
//#include "GameFramework/Actor.h"
//
//class FQuadTreeStorage : public IUnitStorageStrategy
//{
//public:
//    FQuadTreeStorage(const FBox2D& InBounds, int32 InMaxDepth = 5, int32 InMaxObjectsPerNode = 10);
//    virtual ~FQuadTreeStorage() = default;
//
//    virtual void AddUnit(AActor* Unit, ETeam Team) override;
//    virtual void RemoveUnit(AActor* Unit) override;
//    virtual AActor* FindNearestUnit(const FVector2D& Position, ETeam EnemyTeam) const override;
//    virtual void UpdateUnit(AActor* Unit, const FVector2D& NewPosition) override;
//
//private:
//    struct FQuadTreeNode
//    {
//        FBox2D Bounds;
//        int32 Depth;
//        TArray<AActor*> Objects;
//        TUniquePtr<FQuadTreeNode> Children[4];
//        bool bIsLeaf;
//
//        FQuadTreeNode(const FBox2D& InBounds, int32 InDepth)
//            : Bounds(InBounds), Depth(InDepth), bIsLeaf(true)
//        {}
//
//        void Subdivide();
//        bool Insert(AActor* Unit, const FVector2D& Pos, int32 MaxDepth, int32 MaxObjectsPerNode);
//        bool Remove(AActor* Unit, const FVector2D& Pos);
//        void Query(const FVector2D& Position, float& ClosestDistSq, AActor*& ClosestUnit) const;
//    };
//
//    TUniquePtr<FQuadTreeNode> RootNodes[2]; // 0 = Friendly, 1 = Enemy
//
//    int32 MaxDepth;
//    int32 MaxObjectsPerNode;
//
//    TMap<AActor*, ETeam> UnitToTeamMap;
//    TMap<AActor*, FVector2D> UnitToPositionMap;
//
//    // Hilfsfunktion zum Einfügen
//    void InsertUnit(AActor* Unit, ETeam Team);
//
//    // Hilfsfunktion zum Entfernen
//    void RemoveUnitFromTree(AActor* Unit, ETeam Team);
//
//    // Hilfsfunktion zum Finden
//    AActor* FindNearestInTeam(const FVector2D& Position, ETeam Team) const;
//};
//
//
////#pragma once
////
////#include "IUnitStorageStrategy.h"
////#include "CoreMinimal.h"
////
////class FQuadTreeNode;
////
////class FQuadTreeStorage : public IUnitStorageStrategy
////{
////public:
////    FQuadTreeStorage(const FVector2D& InOrigin = FVector2D(0, 0), float InExtent = 10000.f, int32 InMaxUnitsPerNode = 8);
////    virtual ~FQuadTreeStorage();
////
////    virtual void AddUnit(AActor* Unit) override;
////    virtual void RemoveUnit(AActor* Unit) override;
////    virtual AActor* FindNearestUnit(const FVector2D& Position) const override;
////
////private:
////    TUniquePtr<FQuadTreeNode> RootNode;
////};
