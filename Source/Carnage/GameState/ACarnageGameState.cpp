// ACarnageGameState.cpp
#include "ACarnageGameState.h"

#include "Net/UnrealNetwork.h"

#include "enum/EFaction.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"
#include "../GameMode/ACarnageGameMode.h"
#include "../GameMode/EReadyComponent.h"

#include "UFactionState.h"


int32 ACarnageGameState::NextUnitId = 0;

ACarnageGameState::ACarnageGameState() {
    // Set this pawn to call Tick() every frame.  
    PrimaryActorTick.bCanEverTick = true;

    this->mSpatialStorageManager = CreateDefaultSubobject<URTSUnitManagerComponent>(TEXT("UnitManager"));
}

int32 ACarnageGameState::GetAllianceCount() const
{
    return this->FArrayAlliances.Num();;
}

UFactionState* ACarnageGameState::GetFactionByIndex(int32 Index) const
{
    return FArrayFactions.IsValidIndex(Index) ? FArrayFactions[Index] : nullptr;
}

UFactionState* ACarnageGameState::GetFactionById(EFaction factionId) {
    for (UFactionState* faction : FArrayFactions) {
        if (faction->GetFactionId() == factionId) {
            return faction;
        }
    }

    return nullptr;
}

UFactionState* ACarnageGameState::GetPlayerFaction() const
{
    //return GetFactionByIndex(0);
    for(UFactionState* faction : FArrayFactions) {
        if (faction->GetFactionId() == this->playerFactionId) {
            return faction;
        }
    }

    return nullptr;
}

int32 ACarnageGameState::GetFactionCount() const
{
    return FArrayFactions.Num();
}

void ACarnageGameState::BeginPlay()
{    
    Super::BeginPlay();

    this->playerFactionId = EFaction::Faction_1;

    if (FArrayFactions.Num() == 0)
    {
        // Player faction (Index 0)
        //UFactionState* PlayerFaction = NewObject<UFactionState>(this,)
        UFactionState* PlayerFaction = NewObject<UFactionState>(this);

        PlayerFaction->AddResources(1234678);
        FArrayFactions.Add(PlayerFaction);

        // Enemy faction (Index 1)
        UFactionState* AlienFaction = NewObject<UFactionState>(this);
        AlienFaction->AddResources(1000000);
        FArrayFactions.Add(AlienFaction);

        //Adding them to different Alliances makes them enemys
        if (FArrayAlliances.Num() == 0) {
            UAlliance* AllianceA = NewObject<UAlliance>(this);
            AllianceA->SetAllianceId(EAlliance::Alliance_A);
            AllianceA->AddFaction(PlayerFaction);
            FArrayAlliances.Add(AllianceA);

            UAlliance* AllianceB = NewObject<UAlliance>(this);
            AllianceB->SetAllianceId(EAlliance::Alliance_B);
            AllianceB->AddFaction(AlienFaction);
            FArrayAlliances.Add(AllianceB);
        }
    }

    if(ACarnageGameMode* GM = GetWorld()->GetAuthGameMode<ACarnageGameMode>())
    {
        GM->RegisterReadyComponent(EReadyComponent::GameState, this);
    }
    else {
        UE_LOG(LogTemp, Error, TEXT("ACarnageGameState::BeginPlay() - !!! CarnageGameMode Not Ready !!!"));
    }
}

void ACarnageGameState::Tick(float DeltaTime)
{
    for (int i = 0; i < FArrayFactions.Num();i++) {
        FArrayFactions[i]->TickProduction(DeltaTime);
    }
}

int32 ACarnageGameState::GetNextUnitId()
{
    return NextUnitId++;
}

void ACarnageGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACarnageGameState, playerFactionId);
}

