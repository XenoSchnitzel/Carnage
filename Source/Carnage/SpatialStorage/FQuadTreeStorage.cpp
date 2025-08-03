
#include "FQuadTreeStorage.h"
#include "GameFramework/Actor.h"
#include "../GameState/enum/EFaction.h"
#include "Math/UnrealMathUtility.h"

FQuadTreeStorage::FQuadTreeNode::FQuadTreeNode(const FBox2D& InBounds, int32 InDepth)
    : Bounds(InBounds), Depth(InDepth), bIsLeaf(true)
{
}

void FQuadTreeStorage::FQuadTreeNode::Subdivide()
{
    FVector2D Center = Bounds.GetCenter();
    FVector2D Min = Bounds.Min;
    FVector2D Max = Bounds.Max;

    Children[0] = MakeUnique<FQuadTreeNode>(FBox2D(Min, Center), Depth + 1);
    Children[1] = MakeUnique<FQuadTreeNode>(FBox2D(FVector2D(Center.X, Min.Y), FVector2D(Max.X, Center.Y)), Depth + 1);
    Children[2] = MakeUnique<FQuadTreeNode>(FBox2D(FVector2D(Min.X, Center.Y), FVector2D(Center.X, Max.Y)), Depth + 1);
    Children[3] = MakeUnique<FQuadTreeNode>(FBox2D(Center, Max), Depth + 1);

    bIsLeaf = false;

    // Reinsert objects to children
    for (AActor* Obj : Objects)
    {
        FVector2D Pos(Obj->GetActorLocation().X, Obj->GetActorLocation().Y);
        for (int32 i = 0; i < 4; ++i)
        {
            if (Children[i]->Bounds.IsInside(Pos))
            {
                Children[i]->Objects.Add(Obj);
                break;
            }
        }
    }
    Objects.Empty();
}

bool FQuadTreeStorage::FQuadTreeNode::Insert(AActor* Unit, const FVector2D& Pos, int32 MaxDepth, int32 MaxObjectsPerNode)
{
    if (!Bounds.IsInside(Pos))
        return false;

    if (bIsLeaf)
    {
        Objects.Add(Unit);
        if (Objects.Num() > MaxObjectsPerNode && Depth < MaxDepth)
        {
            Subdivide();
        }
        return true;
    }
    else
    {
        for (int32 i = 0; i < 4; ++i)
        {
            if (Children[i]->Insert(Unit, Pos, MaxDepth, MaxObjectsPerNode))
                return true;
        }
    }
    return false;
}

bool FQuadTreeStorage::FQuadTreeNode::Remove(AActor* Unit, const FVector2D& Pos)
{
    if (!Bounds.IsInside(Pos))
        return false;

    if (bIsLeaf)
    {
        return Objects.Remove(Unit) > 0;
    }
    else
    {
        for (int32 i = 0; i < 4; ++i)
        {
            if (Children[i]->Remove(Unit, Pos))
                return true;
        }
    }
    return false;
}

void FQuadTreeStorage::FQuadTreeNode::Query(const FVector2D& Position, float& ClosestDistSq, AActor*& ClosestUnit) const
{
    if (bIsLeaf)
    {
        for (AActor* Unit : Objects)
        {
            FVector2D UnitPos(Unit->GetActorLocation().X, Unit->GetActorLocation().Y);
            float DistSq = FVector2D::DistSquared(UnitPos, Position);
            if (DistSq < ClosestDistSq)
            {
                ClosestDistSq = DistSq;
                ClosestUnit = Unit;
            }
        }
    }
    else
    {
        // Sort children by distance to position
        TArray<int32> ChildOrder = { 0,1,2,3 };
        ChildOrder.Sort([&](int32 A, int32 B) {
            FVector2D CenterA = Children[A]->Bounds.GetCenter();
            FVector2D CenterB = Children[B]->Bounds.GetCenter();
            return FVector2D::DistSquared(CenterA, Position) < FVector2D::DistSquared(CenterB, Position);
            });

        for (int32 i : ChildOrder)
        {
            if (!Children[i]) continue;

            FVector2D ClosestPointInBox = Children[i]->Bounds.GetClosestPointTo(Position);
            float DistToBoxSq = FVector2D::DistSquared(Position, ClosestPointInBox);
            if (DistToBoxSq > ClosestDistSq)
                continue;

            Children[i]->Query(Position, ClosestDistSq, ClosestUnit);
        }
    }
}

FQuadTreeStorage::FQuadTreeStorage(const FBox2D& InBounds, int32 InMaxDepth, int32 InMaxObjectsPerNode)
    : MaxDepth(InMaxDepth), MaxObjectsPerNode(InMaxObjectsPerNode)
{
    RootNodes[static_cast<int32>(EFaction::Faction_1)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_2)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_3)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_4)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_5)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_6)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_7)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
    RootNodes[static_cast<int32>(EFaction::Faction_8)] = MakeUnique<FQuadTreeNode>(InBounds, 0);
}

void FQuadTreeStorage::AddUnit(AActor* Unit, EFaction myTeam)
{
    if (!Unit) return;

    FVector2D Pos(Unit->GetActorLocation().X, Unit->GetActorLocation().Y);
    int32 TeamIndex = static_cast<int32>(myTeam);

    if (RootNodes[TeamIndex] && RootNodes[TeamIndex]->Insert(Unit, Pos, MaxDepth, MaxObjectsPerNode))
    {
        UnitToTeamMap.Add(Unit, myTeam);
        UnitToPositionMap.Add(Unit, Pos);
    }
}

void FQuadTreeStorage::RemoveUnit(AActor* Unit)
{
    if (!Unit) return;

    if (const EFaction* TeamPtr = UnitToTeamMap.Find(Unit))
    {
        int32 TeamIndex = static_cast<int32>(*TeamPtr);
        FVector2D Pos = UnitToPositionMap.FindRef(Unit);

        if (RootNodes[TeamIndex])
        {
            RootNodes[TeamIndex]->Remove(Unit, Pos);
            UnitToTeamMap.Remove(Unit);
            UnitToPositionMap.Remove(Unit);
        }
    }
}

AActor* FQuadTreeStorage::FindNearestUnit(const FVector2D& Position, EFaction myTeam) const
{
    int32 TeamIndex = static_cast<int32>(myTeam);
    if (!RootNodes[TeamIndex]) return nullptr;

    float ClosestDistSq = TNumericLimits<float>::Max();
    AActor* ClosestUnit = nullptr;

    RootNodes[TeamIndex]->Query(Position, ClosestDistSq, ClosestUnit);
    return ClosestUnit;
}

void FQuadTreeStorage::UpdateUnit(AActor* Unit, const FVector2D& NewPosition)
{
    if (!Unit) return;

    FVector2D OldPosition = UnitToPositionMap.FindRef(Unit);
    float DistSq = FVector2D::DistSquared(OldPosition, NewPosition);

    // Optional Threshold zum Vermeiden unnötiger Updates (z.B. 1cm)
    if (DistSq > KINDA_SMALL_NUMBER)
    {
        if (const EFaction* TeamPtr = UnitToTeamMap.Find(Unit))
        {
            int32 TeamIndex = static_cast<int32>(*TeamPtr);

            // Entfernen + Neu Einfügen
            if (RootNodes[TeamIndex])
            {
                RootNodes[TeamIndex]->Remove(Unit, OldPosition);
                RootNodes[TeamIndex]->Insert(Unit, NewPosition, MaxDepth, MaxObjectsPerNode);

                UnitToPositionMap[Unit] = NewPosition;
            }
        }
    }
}