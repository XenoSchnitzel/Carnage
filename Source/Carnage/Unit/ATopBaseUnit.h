#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "UAttackComponent.h"
#include "UHitpointComponent.h"
#include "EUnitStates.h"

#include <Carnage\Resources\AResourceNode.h>

#include "ATopBaseUnit.generated.h"


// ADD (above the class, after includes)
struct FAIRequestID;
struct FPathFollowingResult;

// Delegate that broadcasts when this unit dies
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

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

	/** Clear references, remove delegates, ...*/
	void InvalidateAttackTarget();

	/** Clear references, remove delegates, ...*/
	void InvalidateMiningTarget();


#pragma region State_Machine



	void State_Idle(float DeltaSeconds);
	void State_Moving(float DeltaSeconds);
	void State_Mining(float DeltaSeconds);

	void State_Mining_MoveTo(float DeltaSeconds);
	void State_Mining_MoveFrom(float DeltaSeconds);
	void State_Mining_At(float DeltaSeconds);

	void State_Attacking(float DeltaSeconds);

	void State_Attack_Rotate(float DeltaSeconds);
	void State_Attack_Start(float DeltaSeconds);
	void State_Attack_Performing(float DeltaSeconds);
	void State_Attack_CooldownStart(float DeltaSeconds);
	void State_Attack_CooldownPerfoming(float DeltaSeconds);

	void State_Idle_Chilling(float DeltaSeconds);
	void IdleCooldownState(float DeltaSeconds);


	//TODO: Add a helper function that checks times for switching states: bool StateOverDuration(State);

#pragma endregion

protected:

	// State changes always set this counter to zero
	UPROPERTY(BlueprintReadOnly, Category = "State")
	float p_fStateTimeCounter = 0.0f;

	UPROPERTY(BlueprintGetter = IsSelected, meta = (AllowPrivateAccess = "true"))
	bool bIsSelected = false;

	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsSelected() const { return bIsSelected; }

	UPROPERTY(BlueprintGetter = IsPreSelected, meta = (AllowPrivateAccess = "true"))
	bool bIsPreSelected = false;

	UFUNCTION(BlueprintPure, Category = "Selection")
	bool IsPreSelected() const { return bIsPreSelected; }

	// --- Fast set of overlapping units (O(1) add/remove, order not guaranteed) ---
	UPROPERTY(VisibleAnywhere, Category = "Overlap")
	TSet<TWeakObjectPtr<ATopBaseUnit>> OverlappingUnits;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UAttackComponent* AttackComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHitpointComponent* HitpointComponent;

	/** Altenative to UE collision detection, close units are "damped" away like a spring is in between*/
	UFUNCTION(BlueprintCallable, Category = "Overlap")
	void DampOverlappingUnits(float DeltaTime);

	/** Returns closest enemy, its distance and a found flag */
	UFUNCTION(BlueprintCallable, Category = "Combat|Query")
	FClosestEnemyResult GetClosestEnemyUnit() const;

#pragma region Events

private:

	/** Called when the path following component finishes this move request. */
	void OnMoveRequestFinished(struct FAIRequestID RequestID, const FPathFollowingResult& Result);

public:
	/** Move towards the nearest enemy using AIController::MoveToActor (recommended C++ path). */
	UFUNCTION(BlueprintCallable, Category = "Unit|Movement")
	void MoveToNearestEnemy();

	/** Event that gets fired when this unit dies */
	UPROPERTY(BlueprintAssignable, Category = "Unit|Combat")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Events")
	void OnMyDeath();
	virtual void OnMyDeath_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Events")
	void BroadcastOnDeath();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Combat")
	void OnHit(ATopBaseUnit* Attacker);
	virtual void OnHit_Implementation(ATopBaseUnit* Attacker);

	UFUNCTION(BlueprintImplementableEvent, Category = "Unit|Combat")
	void OnAfterDamageApplied(float NewHealth);


	/** Called when another actor starts overlapping this actor */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/** Called when another actor stops overlapping this actor */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual void Tick(float DeltaTime) override;

#pragma endregion

public:

	ATopBaseUnit();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AActor* AttackTarget = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AResourceNode* MiningTarget = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EFaction FactionId;

	// Toggle per instance in Editor/PIE and per Code
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug|StateLog")
	bool bStateLogEnabled = false;

	/** Called when the current attack target dies */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Events")
	 void OnAttackTargetDeath();
	 virtual void OnAttackTargetDeath_Implementation();

	 UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Debug|Attack Indicator")
	 void ShowAttackIndicator(bool show);
	 virtual void ShowAttackIndicator_Implementation(bool show);

	 UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selection")
	 void SelectUnit();
	 virtual void SelectUnit_Implementation();

	 UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selection")
	 void DeSelectUnit();
	 virtual void DeSelectUnit_Implementation();

	 UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selection")
	 void PreSelectUnit();
	 virtual void PreSelectUnit_Implementation();

	 UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Selection")
	 void DePreSelectUnit();
	 virtual void DePreSelectUnit_Implementation();

//mostly intended for blueprint to handle stuff like animation changes, etc.
#pragma region StateChangeHandler 

	 // Kann in Blueprints überschrieben werden, C++ muss nichts implementieren
	 UFUNCTION(BlueprintImplementableEvent, Category = "StateChangeHandler")
	 void StartMovementHandler();

	 //// Kann in Blueprints überschrieben werden, C++ muss nichts implementieren
	 //UFUNCTION(BlueprintImplementableEvent, Category = "StateChangeHandler")
	 //void StartAttackHandler();

	 //// Kann in Blueprints überschrieben werden, C++ muss nichts implementieren
	 //UFUNCTION(BlueprintImplementableEvent, Category = "StateChangeHandler")
	 //void StopAttackHandler();

#pragma endregion


#pragma region Commands

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Movement")
	void Command_StartAttack(ATopBaseUnit* target);
	virtual void Command_StartAttack_Implementation(ATopBaseUnit* attackTarget);

	UFUNCTION(BlueprintCallable, Category = "Unit|Movement")
	void Command_Stop();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Movement")
	void Command_MoveTo(const FVector& NewPos);
	virtual void Command_MoveTo_Implementation(const FVector& NewPos); 

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Movement")
	void Command_MineResource(AResourceNode* miningNode);
	virtual void Command_MineResource_Implementation(AResourceNode* miningNode);

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void SetUnitState(EUnitMakroState makroState, EUnitMikroState mikroState);

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMakroState GetUnitMakroState() const;

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMikroState GetUnitMikroState() const;

#pragma region helpers

	/** This method handles attacking another unit, including calling the hit event */
	UFUNCTION(BlueprintCallable, Category = "Unit|Combat")
	bool TryAttackTarget();

	/** Tries to auto attack the closest enemy if within minimum range. Returns true if attack started */
	UFUNCTION(BlueprintCallable, Category = "Unit|Combat")
	bool TryAutoAttackIfTargetIsWithinMinimumRange();

	/** True if our current facing (2D) is almost pointing at AttackTarget (dot >= 0.995) */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Unit|Combat")
	bool IsFacingAttackTarget() const;

	/** Rotate smoothly towards current AttackTarget. */
	UFUNCTION(BlueprintCallable, Category = "Unit|Combat")
	void RotateToAttackTarget(float DeltaTime);

#pragma endregion
	
};