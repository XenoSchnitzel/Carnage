// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "UAttackComponent.h"
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

#pragma region State_Machine

	//State changes always set this counter to zero
	float p_fStateTimeCounter = 0.0f;

	void IdleState(float DeltaSeconds);
	void MovingState(float DeltaSeconds);
	void AttackingState(float DeltaSeconds);

	void AttackRotateState(float DeltaSeconds);
	void AttackStartState(float DeltaSeconds);
	void AttackPerfomingState(float DeltaSeconds);
	void AttackCooldownStartState(float DeltaSeconds);
	void AttackCooldownPeformingState(float DeltaSeconds);


	//TODO: implement this a general timer for switching states: bool StateOverDuration(State);

#pragma endregion

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttackComponent* AttackComponent;

#pragma region State_Machine

	virtual void Tick(float DeltaSeconds) override;

#pragma endregion

public:

	ATopBaseUnit();

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