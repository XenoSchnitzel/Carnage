#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"
#include "UAttackComponent.h"
#include "EUnitStates.h"

#include "ATopBaseUnit.generated.h"

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
	void InvalidateTarget();

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Selection")
	bool bIsSelected = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Selection")
	bool bIsPreSelected = false;

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

	/** Event that gets fired when this unit dies */
	UPROPERTY(BlueprintAssignable, Category = "Unit|Combat")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Events")
	void OnMyDeath();
	virtual void OnMyDeath_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Events")
	void BroadcastOnDeath();


	/** Called when another actor starts overlapping this actor */
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	/** Called when another actor stops overlapping this actor */
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	virtual void Tick(float DeltaTime) override;

#pragma endregion

public:

	ATopBaseUnit();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	AActor* AttackTarget = nullptr;

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EFaction FactionId;

	/** Called when the current attack target dies */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Events")
	 void OnAttackTargetDeath();
	 virtual void OnAttackTargetDeath_Implementation();

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


#pragma region Commands

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Movement")
	void StartAttackCommand(ATopBaseUnit* target);
	virtual void StartAttackCommand_Implementation(ATopBaseUnit* attackTarget);

	UFUNCTION(BlueprintCallable, Category = "Unit|Movement")
	void StopCommand();

	// von BP UND C++ überschreibbar
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Unit|Movement")
	void MoveToCommand(const FVector& NewPos);
	virtual void MoveToCommand_Implementation(const FVector& NewPos); 

#pragma endregion

	UFUNCTION(BlueprintCallable)
	void SetUnitState(EUnitMakroState makroState, EUnitMikroState mikroState);

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMakroState GetUnitMakroState() const;

	UFUNCTION(BlueprintGetter, Category = "__State")
	EUnitMikroState GetUnitMikroState() const;
	
};