// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "UAttackComponent.h"
#include "EUnitStates.h"


#include "ATopBaseUnit.generated.h"


/** Result of GetClosestEnemyUnit */
USTRUCT(BlueprintType)
struct FClosestEnemyResult
{
	GENERATED_BODY()

	/** Closest enemy unit (nullptr if none) */
	UPROPERTY(BlueprintReadOnly)
	ATopBaseUnit* ClosestEnemy = nullptr;

	/** Distance in world units (0.0f if none) */
	UPROPERTY(BlueprintReadOnly)
	float ClosestEnemyDistance = 0.0f;

	/** True if an enemy was found */
	UPROPERTY(BlueprintReadOnly)
	bool EnemyFound = false;
};

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

	// --- Fast set of overlapping units (O(1) add/remove, order not guaranteed) ---
	UPROPERTY(VisibleAnywhere, Category = "Overlap")
	TSet<TWeakObjectPtr<ATopBaseUnit>> OverlappingUnits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttackComponent* AttackComponent;
	
	/** Altenative to UE collision detection, close units are "damped" away like a spring is in between*/
	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void DampOverlappingUnits(float DeltaTime);

	/** Returns closest enemy, its distance and a found flag */
	UFUNCTION(BlueprintCallable, Category = "Combat|Query")
	FClosestEnemyResult GetClosestEnemyUnit() const;

#pragma region Events

	/** Called when another actor starts overlapping this actor */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/** Called when another actor stops overlapping this actor */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

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