#pragma once

#include "CoreMinimal.h"
#include "IUnitStorageStrategy.h"
#include "../GameState/enum/EFaction.h"
#include "GameFramework/Actor.h"

class FQuadTreeStorage : public IUnitStorageStrategy
{
public:
    FQuadTreeStorage(const FBox2D& InBounds, int32 InMaxDepth = 5, int32 InMaxObjectsPerNode = 10);
    virtual ~FQuadTreeStorage() override = default;

    virtual void AddUnit(AActor* Unit, EFaction myTeam) override;
    virtual void RemoveUnit(AActor* Unit) override;
    virtual AActor* FindNearestUnit(const FVector2D& Position, EFaction myTeam) const override;
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

    TMap<AActor*, EFaction> UnitToTeamMap;
    TMap<AActor*, FVector2D> UnitToPositionMap;

    //void InsertUnit(AActor* Unit, EFaction myTeam);
    //void RemoveUnitFromTree(AActor* Unit, EFaction myTeam);
    //AActor* FindNearestInTeam(const FVector2D& Position, EFaction myTeam) const;
};