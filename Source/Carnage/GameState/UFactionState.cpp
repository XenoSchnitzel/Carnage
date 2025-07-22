#include "UFactionState.h"
#include "UnitBuildDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "ACarnageGameState.h"

void UFactionState::EnqueueProduction(ECarnageUnitType UnitType)
{
    if (UUnitBuildDataSubsystem* BuildData = UGameplayStatics::GetGameInstance(this)->GetSubsystem<UUnitBuildDataSubsystem>())
    {
        const float BuildTime = BuildData->GetBuildTimeForUnit(UnitType);
        if (BuildTime <= 0.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid BuildTime for unit type!"));
            return;
        }

        FProductionOrder NewOrder;
        NewOrder.UnitType = UnitType;
        NewOrder.BuildTime = BuildTime;
        NewOrder.UnitId = ACarnageGameState::GetNextUnitId();
        //UE_LOG(LogTemp, Warning, TEXT("UFactionState::EnqueueProduction New GUID: %i"), NewOrder.UnitId);
        ProductionQueue.Add(NewOrder);
        NewProductionOrderQueued.Broadcast(NewOrder); // 🔥 Fire production queue event to UI

        //UE_LOG(LogTemp, Warning, TEXT("UFactionState::EnqueueProduction NewProductionOrderQueued"));
        if (ProductionQueue.Num() == 1) {
            FProductionOrder& Next = ProductionQueue[0];
            ProductionOrderStarted.Broadcast(Next); // 🔥 Fire production started event to UI
            //UE_LOG(LogTemp, Warning, TEXT("UFactionState::EnqueueProduction Production-Order-Started"));
        }
    }
}

void UFactionState::TickProduction(float DeltaTime)
{
    if (ProductionQueue.Num() == 0) return;

    FProductionOrder& Current = ProductionQueue[0];
    Current.ElapsedTime += DeltaTime;

    if (Current.ElapsedTime >= Current.BuildTime)
    {
        FProductionOrder OrderToBeSent;
        OrderToBeSent.UnitType = Current.UnitType;
        OrderToBeSent.BuildTime = Current.BuildTime;
        OrderToBeSent.UnitId = Current.UnitId; 

        //UE_LOG(LogTemp, Warning, TEXT("UFactionState::TickProduction Production-Order-Finished"));
        ProductionOrderFinished.Broadcast(OrderToBeSent); // 🔥 Fire production finished event to UI
        ProductionQueue.RemoveAt(0); // FIFO
       
        if (ProductionQueue.Num() > 0) {
            Current = ProductionQueue[0];
            FProductionOrder OrderStarted; 
            OrderStarted.UnitType = Current.UnitType;
            OrderStarted.BuildTime = Current.BuildTime;
            OrderStarted.UnitId = Current.UnitId;
            ProductionOrderStarted.Broadcast(OrderStarted); // 🔥 Fire production started event to UI
            //UE_LOG(LogTemp, Warning, TEXT("UFactionState::TickProduction Production-Order-Started"));
        }
    }
}

bool UFactionState::HasTechnology(FName TechID) const
{
    return UnlockedTechnologies.Contains(TechID);
}

void UFactionState::UnlockTechnology(FName TechID)
{
    UnlockedTechnologies.Add(TechID);
}

void UFactionState::RegisterMainBaseBuilding(AActor* mainBase) {
    MainBaseBuilding = mainBase;
}