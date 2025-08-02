#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ProductionOrder.h"
#include "enum/EFaction.h"
#include "enum/EAlliance.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"

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

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EFaction eFactionId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EAlliance eAllianceId;

    // R E S O U R C E S

    // Resource amount (e.g. energy, ore)
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 Resources = 0;

    // U N I T S

    // List of owned units
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TArray<AActor*> UnitList;

    // B U I L D I N G S

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    AActor* MainBaseBuilding;

    // T E C H N O L O G Y

    // Set of unlocked technologies
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TSet<FName> UnlockedTechnologies;

    // P R O D U C T I O N


    UPROPERTY(BlueprintReadWrite, EditAnywhere)
        TArray<FProductionOrder> ProductionQueue;



public:


    EAlliance GetAllianceId() const;
    void SetAllianceId(EAlliance newAllianceId);
    EFaction GetFactionId() const;
    void SetFactionId(EFaction newFactionId);
    

    // R E S O U R C E S

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


    // U N I T S

    UFUNCTION(BlueprintCallable)
        void RegisterUnit(AActor* unit);

    UFUNCTION(BlueprintCallable)
        void UnregisterUnit(AActor* unit);

    // B U I L D I N G S

    UFUNCTION(BlueprintCallable)
        void RegisterMainBaseBuilding(AActor* mainBase);
    
    UFUNCTION(BlueprintCallable)
        AActor* GetMainBaseBuilding() const { return MainBaseBuilding; }

    // T E C H N O L O G Y

    UFUNCTION(BlueprintCallable)
        bool HasTechnology(FName TechID) const;

    UFUNCTION(BlueprintCallable)
        void UnlockTechnology(FName TechID);

    // P R O D U C T I O N

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
