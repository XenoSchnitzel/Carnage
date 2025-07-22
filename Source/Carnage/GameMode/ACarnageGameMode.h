// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EReadyComponent.h"
#include "ACarnageGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllCentralActorsRegistered);

UCLASS(minimalapi)
class ACarnageGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACarnageGameMode();

	UFUNCTION(BlueprintCallable, Category = "Initialization")
void RegisterReadyComponent(EReadyComponent Type, UObject* Sender);

protected:
	virtual void BeginPlay() override;

private:
	TSet<EReadyComponent> ReadyComponents;
	void TryFinalizeInitialization();

	UPROPERTY(BlueprintAssignable)
		FAllCentralActorsRegistered AllCentralActorsRegistered;

};



