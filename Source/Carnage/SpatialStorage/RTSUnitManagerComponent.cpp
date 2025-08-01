#include "RTSUnitManagerComponent.h"
#include "FSpatialHashStorage.h"
#include "FQuadTreeStorage.h"
#include "../GameState/enum/EHostility.h"

URTSUnitManagerComponent::URTSUnitManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URTSUnitManagerComponent::BeginPlay()
{
    Super::BeginPlay();

    StorageStrategy = MakeUnique<FSpatialHashStorage>(1000.0f);

    bIsReady = true;

    //work down Deferred Registrations 
    for (const auto& Pair : DeferredRegistrations)
    {
        StorageStrategy->AddUnit(Pair.Key, Pair.Value);
    }

    DeferredRegistrations.Empty();
}

void URTSUnitManagerComponent::RegisterUnit(AActor* Unit, EHostility Hostility)
{
    if (!bIsReady)
    {
        DeferredRegistrations.Add(TPair<AActor*, EHostility>(Unit, Hostility));
    }
    else
    {
        if (StorageStrategy)
        {
            StorageStrategy->AddUnit(Unit, Hostility);
        }
    }
}

void URTSUnitManagerComponent::UnregisterUnit(AActor* Unit)
{
    if (StorageStrategy)
    {
        StorageStrategy->RemoveUnit(Unit);
    }
}

AActor* URTSUnitManagerComponent::GetClosestEnemyUnit(const FVector2D& Position, EHostility Hostility) const
{
    if (StorageStrategy)
    {
        EHostility MyHostility = (Hostility == EHostility::Friendly) ? EHostility::Enemy : EHostility::Friendly;
        return StorageStrategy->FindNearestUnit(Position, MyHostility);
    }

    return nullptr;
}

UFUNCTION(BlueprintCallable, Category = "RTS")
void URTSUnitManagerComponent::UpdateUnit(AActor* Unit, FVector2D position) {
    if (StorageStrategy)
    {
        StorageStrategy->UpdateUnit(Unit, position);
    }
}

//void URTSUnitManagerComponent::BeginPlay()
//{
//    Super::BeginPlay();
//
//    switch (StrategyType)
//    {
//    case EUnitStorageStrategy::SpatialHashing:
//        Strategy = MakeUnique<FSpatialHashStorage>(CellSize);
//        break;
//
//    case EUnitStorageStrategy::Quadtree:
//        Strategy = MakeUnique<FQuadTreeStorage>();
//        break;
//
//    default:
//        UE_LOG(LogTemp, Warning, TEXT("Unknown strategy! Defaulting to SpatialHashing"));
//        Strategy = MakeUnique<FSpatialHashStorage>(CellSize);
//        break;
//    }
//}
//
//void URTSUnitManagerComponent::RegisterUnit(AActor* Unit)
//{
//    if (Strategy) Strategy->AddUnit(Unit);
//}
//
//void URTSUnitManagerComponent::UnregisterUnit(AActor* Unit)
//{
//    if (Strategy) Strategy->RemoveUnit(Unit);
//}
//
//AActor* URTSUnitManagerComponent::GetClosestUnit(const FVector2D& Position) const
//{
//    return Strategy ? Strategy->FindNearestUnit(Position) : nullptr;
//}