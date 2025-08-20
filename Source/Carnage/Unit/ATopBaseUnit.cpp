// Copyright Epic Games, Inc. All Rights Reserved.
#include "ATopBaseUnit.h"
#include "../GameState/ACarnageGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Tasks/AITask_MoveTo.h"

#include "Navigation/PathFollowingComponent.h"   // für EPath

#include "../Logging/StateLogger.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"

ATopBaseUnit::ATopBaseUnit()
{
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
}

void ATopBaseUnit::BeginPlay()
{
	CVarStateSystemLog->Set(1, ECVF_SetByCode); //Enables Makro/Mikro State Changes logging by default 
	
	/* - OR - type in game console:

	r.StateSystem.Log 1   // enable logs
	r.StateSystem.Log 0   // disable logs */ 
	
	Super::BeginPlay();



	if (ACarnageGameState* GameState = Cast<ACarnageGameState>(GetWorld()->GetGameState()))
	{
		GameState->RegisterUnit(this);
	}

	this->ECurrentUnitMakroState = EUnitMakroState::UnitMakroState_Idle;
	this->ECurrentUnitMikroState = EUnitMikroState::UnitMikroState_Idle_Chilling;

	p_fStateTimeCounter = 0.0f;
}

#pragma region Commands

void ATopBaseUnit::StartAttackCommand(ATopBaseUnit* target)
{
}

void ATopBaseUnit::StopCommand()
{
}

void ATopBaseUnit::MoveToCommand_Implementation(const FVector &newPos)
{
	//A movement start invalidates any attack targets
	AttackTarget = nullptr;

	SetUnitState(
		EUnitMakroState::UnitMakroState_Moving,
		EUnitMikroState::UnitMikroState_Move_Direct_Move);

	AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
	if (!AI) { return; }

	// If you don't terminate an ongoing movement, you
	// run into trouble with the AI trying to reach
	// all instructed positions
	const auto MoveStatus = AI->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Moving
		|| MoveStatus == EPathFollowingStatus::Type::Moving)
	{
		AI->StopMovement();
	}

	//Now actually initiate moving task
	UAITask_MoveTo* Task = UAITask_MoveTo::AIMoveTo(
		/* AI Controller*/AI,
		/* Goal Location*/newPos,
		/* Goal Actor */nullptr,
		/* AcceptanceRadius */ 50.f,
		/* Stop on Overlap */EAIOptionFlag::Enable,
		/* Accept Partial Path */EAIOptionFlag::Disable,
		/* Use Pathfindein */true,
		/* Lock all logic*/false,
		/* Use continuos goal tracking*/false,
		/* Projekt goal on navigation*/EAIOptionFlag::Enable);

	Task->ReadyForActivation();

}

#pragma endregion

EUnitMakroState ATopBaseUnit::GetUnitMakroState() const
{
	return ECurrentUnitMakroState;
}

EUnitMikroState ATopBaseUnit::GetUnitMikroState() const
{
	return ECurrentUnitMikroState;
}



void ATopBaseUnit::SetUnitState(EUnitMakroState makroState, EUnitMikroState mikroState)
{
	ECurrentUnitMakroState = makroState;
	ECurrentUnitMikroState = mikroState;

	p_fStateTimeCounter = 0.0f;

	STATE_LOG(Log, "SetUnitState() Makro: %s, Mikro %s",
		*UEnum::GetValueAsString(ECurrentUnitMakroState),
		*UEnum::GetValueAsString(ECurrentUnitMikroState));
}

#pragma region State_Machine

void ATopBaseUnit::IdleState(float DeltaSeconds)
{
}

void ATopBaseUnit::MovingState(float DeltaSeconds)
{
}

void ATopBaseUnit::AttackingState(float DeltaSeconds)
{
	switch (ECurrentUnitMikroState) {
	case EUnitMikroState::UnitMikroState_Attack_Rotate:
		AttackRotateState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Attack_Start:
		AttackStartState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Attack_Performing:
		AttackPerfomingState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Attack_Cooldown_Start:
		AttackCooldownStartState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Attack_Cooldown_Performing:
		AttackCooldownPeformingState(DeltaSeconds);
		break;
	default:
		checkf(false, TEXT("Unhandled MikroState in AttackingState: %s"),
			*UEnum::GetValueAsString(ECurrentUnitMikroState));
		break;
	}
}

void ATopBaseUnit::AttackRotateState(float DeltaSeconds)
{
}

void ATopBaseUnit::AttackStartState(float DeltaSeconds)
{
}

void ATopBaseUnit::AttackPerfomingState(float DeltaSeconds)
{
}

void ATopBaseUnit::AttackCooldownStartState(float DeltaSeconds)
{
}

void ATopBaseUnit::AttackCooldownPeformingState(float DeltaSeconds)
{
}

void ATopBaseUnit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	p_fStateTimeCounter += DeltaTime;

	DampOverlappingUnits(DeltaTime);

	switch (ECurrentUnitMakroState) {
	case EUnitMakroState::UnitMakroState_Idle:
		IdleState(DeltaTime);
		break;
	case EUnitMakroState::UnitMakroState_Moving:
		MovingState(DeltaTime);
		break;
	case EUnitMakroState::UnitMakroState_Attacking:
		AttackingState(DeltaTime);
		break;
	default:
		//checkf(false, TEXT("Unhandled MikroState in AttackingState: %s"),
		//	*UEnum::GetValueAsString(ECurrentUnitMakroState));
		break;
	}
}

#pragma endregion

// --- Returns closest enemy unit, distance and a success flag (matches BP flow) ---
FClosestEnemyResult ATopBaseUnit::GetClosestEnemyUnit() const
{
	FClosestEnemyResult Result; // defaults: nullptr / 0 / false

	// Get GameState and the spatial manager
	const UWorld* World = GetWorld();
	const ACarnageGameState* GS = World ? World->GetGameState<ACarnageGameState>() : nullptr;
	if (!GS || !GS->mSpatialStorageManager)
	{
		return Result; // manager missing
	}

	// Build 2D position from our actor location (BP feeds X/Y to the manager)
	const FVector SelfLoc3D = GetActorLocation();
	const FVector2D SelfLoc2D(SelfLoc3D.X, SelfLoc3D.Y);

	// Ask the manager for the closest enemy actor using our faction
	AActor* FoundActor = GS->mSpatialStorageManager->GetClosestEnemyUnit(SelfLoc2D, FactionId);
	if (!FoundActor)
	{
		return Result; // none found
	}

	// Cast to our base unit class (BP cast to BP_BaseUnit_C)
	if (ATopBaseUnit* FoundUnit = Cast<ATopBaseUnit>(FoundActor))
	{
		Result.ClosestEnemy = FoundUnit;

		// BP computes VSize(EnemyLoc - SelfLoc) → 3D distance
		const float Dist = FVector::Dist(FoundUnit->GetActorLocation(), SelfLoc3D);
		Result.ClosestEnemyDistance = Dist;

		Result.EnemyFound = true;
	}

	return Result;
}

// --- Begin overlap: add to set (O(1)) ---
void ATopBaseUnit::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor && OtherActor != this)
	{
		if (ATopBaseUnit* OtherUnit = Cast<ATopBaseUnit>(OtherActor))
		{
			OverlappingUnits.Add(OtherUnit);
		}
	}
}

// --- End overlap: remove from set (O(1)) ---
void ATopBaseUnit::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);

	if (OtherActor && OtherActor != this)
	{
		if (ATopBaseUnit* OtherUnit = Cast<ATopBaseUnit>(OtherActor))
		{
			OverlappingUnits.Remove(OtherUnit);
		}
	}
}

void ATopBaseUnit::DampOverlappingUnits(float DeltaTime)
{
	// Initially no damping
	FVector OverlapCorrectionVector = FVector::ZeroVector;

	const FVector SelfLoc = GetActorLocation();

	for (const TWeakObjectPtr<ATopBaseUnit>& WeakOther : OverlappingUnits)
	{
		ATopBaseUnit* Other = WeakOther.Get();
		if (!Other || Other == this) continue;

		const FVector OtherLoc = Other->GetActorLocation();
		const FVector Delta = SelfLoc - OtherLoc;        // Subtract_VectorVector
		const float  Dist = Delta.Size();              // VSize

		if (Dist <= KINDA_SMALL_NUMBER) continue;

		// s = (1 / (0.003 * Dist)) * DeltaTime   (genau wie deine Multiply/Divide-Kette)
		const float Scale = (DeltaTime) / (0.003f * Dist);

		// Multiply_VectorVector mit (Scale,Scale,Scale) == skalar * Delta
		OverlapCorrectionVector += Delta * Scale;
	}

	// 2) NewLocation = SelfLoc + OverlapCorrectionVector  und SetActorLocation(bSweep=true)
	const FVector NewLoc = SelfLoc + OverlapCorrectionVector;
	FHitResult SweepHit;
	SetActorLocation(NewLoc, /*bSweep*/ true, &SweepHit, ETeleportType::None);
}