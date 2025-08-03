#pragma once

#include "CoreMinimal.h"
#include "../GameState/enum/EHostility.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "Components/ActorComponent.h"
#include "EUnitStorageStrategy.h"
#include "IUnitStorageStrategy.h"
#include "RTSUnitManagerComponent.generated.h"

class IUnitStorageStrategy;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CARNAGE_API URTSUnitManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URTSUnitManagerComponent();

    UFUNCTION(BlueprintCallable, Category = "RTS")
        void RegisterUnit(AActor* Unit, EFaction Faction);

    UFUNCTION(BlueprintCallable, Category = "RTS")
        void UnregisterUnit(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "RTS")
        AActor* GetClosestEnemyUnit(const FVector2D& Position, EFaction myFaction) const;

    UFUNCTION(BlueprintCallable, Category = "RTS")
        void UpdateUnit(AActor* Unit, FVector2D position);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "RTS|Units")
    EUnitStorageStrategy StrategyType = EUnitStorageStrategy::Quadtree;

    UPROPERTY(EditAnywhere, Category = "RTS|Units")
        float CellSize = 1000.0f;

private:
    TUniquePtr<IUnitStorageStrategy> StorageStrategy;
    bool bIsReady = false;
    TArray<TPair<AActor*, EFaction>> DeferredRegistrations;
};
