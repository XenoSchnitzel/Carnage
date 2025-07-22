#pragma once

#include "CoreMinimal.h"
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
        void RegisterUnit(AActor* Unit, ETeam Team);

    UFUNCTION(BlueprintCallable, Category = "RTS")
        void UnregisterUnit(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "RTS")
        AActor* GetClosestEnemyUnit(const FVector2D& Position, ETeam MyTeam) const;

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
    TArray<TPair<AActor*, ETeam>> DeferredRegistrations;
};


//#pragma once
//
//#include "CoreMinimal.h"
//#include "Components/ActorComponent.h"
//#include "EUnitStorageStrategy.h"
//#include "IUnitStorageStrategy.h"
//#include "RTSUnitManagerComponent.generated.h"
//
//class IUnitStorageStrategy;
//
//UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
//class CARNAGE_API URTSUnitManagerComponent : public UActorComponent
//{
//    GENERATED_BODY()
//
//public:
//    URTSUnitManagerComponent();
//
//    UFUNCTION(BlueprintCallable, Category = "RTS|Units")
//        void RegisterUnit(AActor* Unit);
//
//    UFUNCTION(BlueprintCallable, Category = "RTS|Units")
//        void UnregisterUnit(AActor* Unit);
//
//    UFUNCTION(BlueprintCallable, Category = "RTS|Units")
//        AActor* GetClosestUnit(const FVector2D& Position) const;
//
//protected:
//    virtual void BeginPlay() override;
//
//    UPROPERTY(EditAnywhere, Category = "RTS|Units")
//        EUnitStorageStrategy StrategyType = EUnitStorageStrategy::SpatialHashing;
//
//    UPROPERTY(EditAnywhere, Category = "RTS|Units")
//        float CellSize = 1000.0f;
//
//private:
//    TUniquePtr<IUnitStorageStrategy> Strategy;
//};
