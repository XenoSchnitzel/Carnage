#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "UFactionState.h"
#include "ACarnageGameState.generated.h"

UCLASS()
class CARNAGE_API ACarnageGameState : public AGameStateBase
{
    GENERATED_BODY()


        static int32 NextUnitId;

protected:
    // List of all factions in the game
    UPROPERTY(BlueprintReadOnly)
    TArray<UFactionState*> Factions;

public:
    ACarnageGameState();

    UFUNCTION(BlueprintCallable)
        UFactionState* GetPlayerFaction() const;

    UFUNCTION(BlueprintCallable)
        UFactionState* GetFactionByIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable)
        int32 GetFactionCount() const;

    UFUNCTION(BlueprintCallable)
        static int32 GetNextUnitId();


    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

};