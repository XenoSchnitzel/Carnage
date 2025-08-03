#include "RTSUnitManagerComponent.h"
#include "FSpatialHashStorage.h"
#include "FQuadTreeStorage.h"
#include "../GameState/UFactionState.h"

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
    AActor* returnCandidate = nullptr;

    if (StorageStrategy)
    {
        ACarnageGameState* CarnageGameState = Cast<ACarnageGameState>(GetOwner());
        check(CarnageGameState); 
      
        //Lets start HUUGE
        double closestLength = DBL_MAX;

        TArray<UAlliance*> alliances = CarnageGameState->GetAlliances();
        for (UAlliance* alliance : alliances) {

            if (!alliance->IsFactionInAlliance(myFaction)) {
                for (UFactionState* faction : alliance->GetAllFactions()) {

                    AActor* currentCandidate = StorageStrategy->FindNearestUnit(Position, faction->GetFactionId());
                    
                    if (currentCandidate) {
                        FVector Location = currentCandidate->GetActorLocation();
                        double currentLength = FVector2D(Location.X - Position.X, Location.Y - Position.Y).SquaredLength();
                        if (currentLength < closestLength) {
                            closestLength = currentLength;
                            returnCandidate = currentCandidate;
                        }
                    }
                }
            }
        }
    }

    return returnCandidate;
}

void URTSUnitManagerComponent::UpdateUnit(AActor* Unit, FVector2D position) {
    if (StorageStrategy)
    {
        StorageStrategy->UpdateUnit(Unit, position);
    }
}