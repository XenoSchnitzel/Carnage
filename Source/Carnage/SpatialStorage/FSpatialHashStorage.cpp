#include "FSpatialHashStorage.h"
#include "GameFramework/Actor.h"
#include "Math/UnrealMathUtility.h"

FSpatialHashStorage::FSpatialHashStorage(float InCellSize)
    : CellSize(InCellSize)
{}

FSpatialHashStorage::FInt2DKey FSpatialHashStorage::GetCellForPosition(const FVector2D& Position) const
{
    int32 CellX = FMath::FloorToInt(Position.X / CellSize);
    int32 CellY = FMath::FloorToInt(Position.Y / CellSize);
    return FInt2DKey(CellX, CellY);
}

void FSpatialHashStorage::AddUnit(AActor* Unit, EHostility Team)
{
    if (!Unit) return;
    int32 TeamIndex = static_cast<int32>(Team);
    FVector pos = Unit->GetActorLocation();
    FVector2D Pos2D(pos.X, pos.Y);
    FInt2DKey Cell = GetCellForPosition(Pos2D);
	
     //UE_LOG(LogTemp, Warning, TEXT("AddUnit() %s, Team %i, Pos.X %f, Pos.Y %f, Cell.X %i, Cell.Y %i"), *Unit->GetName(), TeamIndex, pos.X, pos.Y, Cell.X, Cell.Y);

    TeamGrids[TeamIndex].FindOrAdd(Cell).Add(Unit);
    UnitToCellMap.Add(Unit, Cell);
    UnitToTeamMap.Add(Unit, Team);
    UnitToPositionMap.Add(Unit, Pos2D);
}

void FSpatialHashStorage::RemoveUnit(AActor* Unit)
{
    if (!Unit) return;

    //UE_LOG(LogTemp, Warning, TEXT("RemoveUnit()"));

    if (const EHostility* TeamPtr = UnitToTeamMap.Find(Unit))
    {
        int32 TeamIndex = static_cast<int32>(*TeamPtr);

       // UE_LOG(LogTemp, Display, TEXT("Team %I"), TeamIndex);
        if (const FInt2DKey* Cell = UnitToCellMap.Find(Unit))
        {
           // UE_LOG(LogTemp, Display, TEXT("Cell.X %i, Cell.Y %i"), Cell->X, Cell->Y);

            if (TSet<AActor*>* CellSet = TeamGrids[TeamIndex].Find(*Cell))
            {
               //UE_LOG(LogTemp, Display, TEXT("CellSet.size - before removal %i"), CellSet->Num());
                CellSet->Remove(Unit);
                if (CellSet->Num() == 0)
                {
                    TeamGrids[TeamIndex].Remove(*Cell);
                }
               // UE_LOG(LogTemp, Display, TEXT("CellSet.size - after removal %i"), CellSet->Num());
            }
            UnitToCellMap.Remove(Unit);
        }
        UnitToTeamMap.Remove(Unit);
    }
    UnitToPositionMap.Remove(Unit);
}


//AActor * FSpatialHashStorage::FindNearestUnit(const FVector2D & Position, EHostility EnemyTeam) const
//{
//    int32 TeamIndex = static_cast<int32>(EnemyTeam);
//    FInt2DKey CenterCell = GetCellForPosition(Position);
//
//    const int32 MaxSearchRadius = 5;
//    float ClosestDistSq = TNumericLimits<float>::Max();
//    AActor* ClosestUnit = nullptr;
//
//    // Sammelt alle Zellen in Suchreichweite, sortiert nach Abstand zum Zentrum
//    TArray<FInt2DKey> CellsToCheck;
//    for (int32 DX = -MaxSearchRadius; DX <= MaxSearchRadius; ++DX)
//    {
//        for (int32 DY = -MaxSearchRadius; DY <= MaxSearchRadius; ++DY)
//        {
//            CellsToCheck.Add(FInt2DKey(CenterCell.X + DX, CenterCell.Y + DY));
//        }
//    }
//
//    // Optional: Sortiere nach Entfernung zur Mitte, damit zuerst nahe Zellen geprüft werden
//    CellsToCheck.Sort([&](const FInt2DKey& A, const FInt2DKey& B)
//        {
//            float DistASq = FVector2D::DistSquared(FVector2D(A.X, A.Y), FVector2D(CenterCell.X, CenterCell.Y));
//            float DistBSq = FVector2D::DistSquared(FVector2D(B.X, B.Y), FVector2D(CenterCell.X, CenterCell.Y));
//            return DistASq < DistBSq;
//        });
//
//    for (const FInt2DKey& Cell : CellsToCheck)
//    {
//        const TSet<AActor*>* Units = TeamGrids[TeamIndex].Find(Cell);
//        if (!Units) continue;
//
//        for (AActor* Unit : *Units)
//        {
//            FVector2D UnitPos(Unit->GetActorLocation().X, Unit->GetActorLocation().Y);
//            float DistSq = FVector2D::DistSquared(UnitPos, Position);
//            if (DistSq < ClosestDistSq)
//            {
//                ClosestDistSq = DistSq;
//                ClosestUnit = Unit;
//            }
//        }
//    }
//
//    if (ClosestUnit) {
//        UE_LOG(LogTemp, Display, TEXT("ClosestUnit: Gefunden, TeamIndex: %i"), TeamIndex);
//    }
//    else {
//        UE_LOG(LogTemp, Display, TEXT("ClosestUnit: Nicht gefunden, TeamIndex: %i"), TeamIndex);
//    }
//
//    return ClosestUnit;
//}

AActor* FSpatialHashStorage::FindNearestUnit(const FVector2D& Position, EHostility EnemyTeam) const
{
    int32 TeamIndex = static_cast<int32>(EnemyTeam);
    FInt2DKey CenterCell = GetCellForPosition(Position);

    const int32 MaxSearchRadius = 20;
    float ClosestDistSq = TNumericLimits<float>::Max();
    AActor* ClosestUnit = nullptr;

   //UE_LOG(LogTemp, Warning, TEXT("FindNearestUnit() Pos.X %f, Pos.Y %f"), Position.X, Position.Y);
    for (int32 Radius = 0; Radius <= MaxSearchRadius; ++Radius)
    {
        for (int32 DX = -Radius; DX <= Radius; ++DX)
        {
            for (int32 DY = -Radius; DY <= Radius; ++DY)
            {
                if (Radius > 0 && FMath::Abs(DX) < Radius && FMath::Abs(DY) < Radius)
                    continue;

                FInt2DKey Cell(CenterCell.X + DX, CenterCell.Y + DY);
                const TSet<AActor*>* Units = TeamGrids[TeamIndex].Find(Cell);
                if (!Units) continue;

                for (AActor* Unit : *Units)
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
        }
        if (ClosestUnit) 
            break;
    }

    //if (ClosestUnit) {
    //    UE_LOG(LogTemp, Display, TEXT("ClosestUnit: Gefunden, TeamIndex: %i"), TeamIndex);
    //}
    //else {
    //    UE_LOG(LogTemp, Display, TEXT("ClosestUnit: Nicht gefunden, TeamIndex: %i"), TeamIndex);
    //}

    return ClosestUnit;
}

void FSpatialHashStorage::UpdateUnit(AActor* Unit, const FVector2D& NewPosition)
{
    if (!Unit) return;

    FVector2D OldPosition = UnitToPositionMap.FindRef(Unit);
    FInt2DKey OldCell = GetCellForPosition(OldPosition);
    FInt2DKey NewCell = GetCellForPosition(NewPosition);
   // UE_LOG(LogTemp, Warning, TEXT("-- UpdateUnit() -- "));

    if (!(OldCell == NewCell))
    {
       // UE_LOG(LogTemp, Display, TEXT("Old Cell != New Cell(), OlDCell.X %i, OldCelly.Y %i, NewCell.X %i, NewCell.X %i"), OldCell.X, OldCell.Y, NewCell.X, NewCell.Y );
        EHostility* Team = UnitToTeamMap.Find(Unit);
        RemoveUnit(Unit);
        if (Team)
        {
           // UE_LOG(LogTemp, Display, TEXT("UpdateUnit() - AddUnit"));
            AddUnit(Unit, *Team);
        }
    }
    else
    {
       // UE_LOG(LogTemp, Display, TEXT("Old Cell == New Cell(), OlDCell.X %i, OldCelly.Y %i, NewCell.X %i, NewCell.X %i"), OldCell.X, OldCell.Y, NewCell.X, NewCell.Y);
        //UE_LOG(LogTemp, Display, TEXT("UnitToPositionMap(), Unit %s, Pos.X %f, Pos.Y %f"), *Unit->GetName(), NewPosition.X, NewPosition.Y);
        UnitToPositionMap.Add(Unit, NewPosition);
    }

}
