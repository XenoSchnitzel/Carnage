#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"

#include "UFactionState.h"
#include "UAlliance.h"
#include "enum/EFaction.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"
#include "../Unit/ATopBaseUnit.h"

#include "ACarnageGameState.generated.h"

UCLASS()
class CARNAGE_API ACarnageGameState : public AGameStateBase
{
    GENERATED_BODY()


    static int32 NextUnitId;

protected:

    virtual void BeginPlay() override;
    
    UPROPERTY(Replicated)
    EFaction playerFactionId;

    UPROPERTY(BlueprintReadWrite, Replicated)
    TArray<UAlliance*> FArrayAlliances;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // List of all factions present in one session
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<UFactionState*> FArrayFactions;


    UFUNCTION(BlueprintCallable)
    virtual void Tick(float DeltaTime) override;

public:
    ACarnageGameState();

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    URTSUnitManagerComponent* mSpatialStorageManager;

    UFUNCTION(BlueprintCallable)
    int32 GetAllianceCount() const;

    UFUNCTION(BlueprintCallable) const
        TArray<UAlliance*> GetAlliances();

    UFUNCTION(BlueprintCallable)
        UFactionState* GetFactionById(EFaction factionId);

    UFUNCTION(BlueprintCallable)
        UFactionState* GetPlayerFaction() const;

    UFUNCTION(BlueprintCallable)
        UFactionState* GetFactionByIndex(int32 Index) const;

    UFUNCTION(BlueprintCallable)
        int32 GetFactionCount() const;

    UFUNCTION(BlueprintCallable)
        static int32 GetNextUnitId();

    UFUNCTION(BlueprintCallable)
    TArray<UFactionState*> GetAllFactionsOfAlliance(EFaction faction);

    UFUNCTION(BlueprintCallable)
    void RegisterUnit(ATopBaseUnit* unit);

};