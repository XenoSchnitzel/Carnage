#include "RTSUnitManagerComponent.h"
#include "FSpatialHashStorage.h"
#include "FQuadTreeStorage.h"
#include "../GameState/ACarnageGameState.h"

#include "../GameState/enum/EHostility.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"

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

void URTSUnitManagerComponent::RegisterUnit(AActor* Unit, EFaction Faction)
{
    if (!bIsReady)
    {
        DeferredRegistrations.Add(TPair<AActor*, EFaction>(Unit, Faction));
    }
    else
    {
        if (StorageStrategy)
        {
            StorageStrategy->AddUnit(Unit, Faction);
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

AActor* URTSUnitManagerComponent::GetClosestEnemyUnit(const FVector2D& Position, EFaction myFaction) const
{
    if (StorageStrategy)
    {
        ACarnageGameState* CarnageGameState = Cast<ACarnageGameState>(GetOwner());
        check(CarnageGameState); 

        TArray<UAlliance*> alliances = CarnageGameState->GetAlliances();
        for (UAlliance* alliance : alliances) {

        }

        return StorageStrategy->FindNearestUnit(Position, myFaction);
    }

    return nullptr;
}

void URTSUnitManagerComponent::UpdateUnit(AActor* Unit, FVector2D position) {
    if (StorageStrategy)
    {
        StorageStrategy->UpdateUnit(Unit, position);
    }
}