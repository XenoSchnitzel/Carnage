// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "EUnitStates.h"


#include "ATopBaseUnit.generated.h"









//TODO: rename ATopBaseUnit to BaseUnit once full c++ migration has happened.
UCLASS(Blueprintable)
class ATopBaseUnit : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintGetter = GetUnitMakroState, Category = "__State")
	EUnitMakroState ECurrentUnitMakroState;

	UPROPERTY(BlueprintGetter = GetUnitMikroState, Category = "__State")
	EUnitMikroState ECurrentUnitMikroState;



public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EFaction FactionId;

	UFUNCTION(BlueprintCallable)
	void SetUnitState(EUnitMakroState makroState, EUnitMikroState mikroState);

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMakroState GetUnitMakroState() const;

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMikroState GetUnitMikroState() const;
	
};