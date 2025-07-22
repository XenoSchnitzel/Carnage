#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProductionOrder.h"
#include "UFactionState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewProductionOrderQueued, const FProductionOrder&, CurrentOrder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProductionOrderStarted, const FProductionOrder&, CurrentOrder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProductionOrderCanceled, const FProductionOrder&, CurrentOrder);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FProductionOrderFinished, const FProductionOrder&, CurrentOrder);

UCLASS(Blueprintable)
class CARNAGE_API UFactionState : public UObject
{
    GENERATED_BODY()

protected:

    // List of owned units
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TArray<AActor*> OwnedUnits;

    // Set of unlocked technologies
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TSet<FName> UnlockedTechnologies;

    // Resource amount (e.g. energy, ore)
    UPROPERTY(BlueprintReadWrite)
        int32 Resources = 0;

    UPROPERTY(BlueprintReadWrite)
        TArray<FProductionOrder> ProductionQueue;

    UPROPERTY(BlueprintReadWrite)
        AActor* MainBaseBuilding;

public:

    UFUNCTION(BlueprintCallable)
        int32 GetResources() const { return Resources; }

    UFUNCTION(BlueprintCallable)
        void AddResources(int32 Amount) { Resources += Amount; }

    UFUNCTION(BlueprintCallable)
        bool TrySpendResources(int32 Cost)
        {
            if (Resources >= Cost)
            {
                Resources -= Cost;
                return true;
            }
            return false;
        }

    UFUNCTION(BlueprintCallable)
        void RegisterMainBaseBuilding(AActor* mainBase);
    
    UFUNCTION(BlueprintCallable)
        AActor* GetMainBaseBuilding() const { return MainBaseBuilding; }

    UFUNCTION(BlueprintCallable)
        bool HasTechnology(FName TechID) const;

    UFUNCTION(BlueprintCallable)
        void UnlockTechnology(FName TechID);

    UFUNCTION(BlueprintCallable)
        void EnqueueProduction(ECarnageUnitType UnitType);

    UPROPERTY(BlueprintAssignable)
        FNewProductionOrderQueued NewProductionOrderQueued;

    UPROPERTY(BlueprintAssignable)
        FProductionOrderStarted ProductionOrderStarted;

    UPROPERTY(BlueprintAssignable)
        FProductionOrderCanceled ProductionOrderCanceled;

    UPROPERTY(BlueprintAssignable)//UI doesnt need that, might be necessary in the future for soundFX or some other "Ready" markings 
        FProductionOrderFinished ProductionOrderFinished;
    
    void TickProduction(float DeltaTime);
};
