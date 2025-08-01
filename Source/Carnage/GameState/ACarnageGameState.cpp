// ACarnageGameState.cpp
#include "ACarnageGameState.h"
#include "../GameMode/ACarnageGameMode.h"
#include "../GameMode/EReadyComponent.h"
#include "UFactionState.h"
#include "Net/UnrealNetwork.h"

int32 ACarnageGameState::NextUnitId = 0;

ACarnageGameState::ACarnageGameState() {
    // Set this pawn to call Tick() every frame.  
    PrimaryActorTick.bCanEverTick = true;
}

UFactionState* ACarnageGameState::GetFactionByIndex(int32 Index) const
{
    return Factions.IsValidIndex(Index) ? Factions[Index] : nullptr;
}

UFactionState* ACarnageGameState::GetPlayerFaction() const
{
    return GetFactionByIndex(0);
}

int32 ACarnageGameState::GetFactionCount() const
{
    return Factions.Num();
}

void ACarnageGameState::BeginPlay()
{    
    Super::BeginPlay();

    if (Factions.Num() == 0)
    {
        // Player faction (Index 0)
        UFactionState* PlayerFaction = NewObject<UFactionState>(this);
        PlayerFaction->AddResources(1234678);
        Factions.Add(PlayerFaction);

        // Enemy faction (Index 1)
        UFactionState* EnemyFaction = NewObject<UFactionState>(this);
        EnemyFaction->AddResources(1000000);
        Factions.Add(EnemyFaction);
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
    for (int i = 0; i < Factions.Num();i++) {
        Factions[i]->TickProduction(DeltaTime);
    }
}

int32 ACarnageGameState::GetNextUnitId()
{
    return NextUnitId++;
}

void ACarnageGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACarnageGameState, myFactionId);
}

