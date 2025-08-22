#include "ATopBaseUnit.h"
#include "../GameState/ACarnageGameState.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Tasks/AITask_MoveTo.h"
#include "AITypes.h"  // for FAIRequestID

#include "Navigation/PathFollowingComponent.h"   // for EPath

#include "../Logging/StateLogger.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"
#include "EUnitStates.h"



#pragma region Construction

ATopBaseUnit::ATopBaseUnit()
{
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	HitpointComponent = CreateDefaultSubobject<UHitpointComponent>(TEXT("HitpointComponent"));
}

void ATopBaseUnit::BeginPlay()
{
	//CVarStateSystemLog->Set(1, ECVF_SetByCode); //Enables Makro/Mikro State Changes logging by default 
	
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

#pragma endregion

#pragma region events

void ATopBaseUnit::OnHit_Implementation(ATopBaseUnit* Attacker)
{
	// Invariants by design:
	// - Attacker != nullptr
	// - Attacker->AttackComponent != nullptr
	// - this->HitpointComponent != nullptr

	const float Damage = Attacker->AttackComponent->Value;

	// Apply damage directly to our C++ health component
	HitpointComponent->Health -= Damage;
	const float NewHealth = HitpointComponent->Health;

	// Optional BP hook (VFX/SFX/UI)
	OnAfterDamageApplied(NewHealth);

	// Handle death transition
	if (NewHealth <= 0.f)
	{
		SetUnitState(EUnitMakroState::UnitMakroState_Dead,
			EUnitMikroState::UnitMikroState_NoSubState);

		SetActorEnableCollision(false);

		if (AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this))
		{
			AI->StopMovement();
		}

		OnMyDeath();
		BroadcastOnDeath();
	}
}

void ATopBaseUnit::SelectUnit_Implementation()
{
	bIsSelected = true;
}

void ATopBaseUnit::DeSelectUnit_Implementation()
{
	bIsSelected = false;
}

void ATopBaseUnit::PreSelectUnit_Implementation()
{
	bIsPreSelected = true;
}

void ATopBaseUnit::DePreSelectUnit_Implementation()
{
	bIsPreSelected = false;
}

void ATopBaseUnit::OnMyDeath_Implementation()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (PC)
	{
		//TODO: This came by Chatty during C++ conversion and needs to be changed into either
		//	a) An interface call
		//  b) Partial transfer of the Player controller into C++ as well

		static const FName FuncName(TEXT("Event UnitDeath"));
		if (UFunction* Func = PC->FindFunction(FuncName))
		{
			struct FEventUnitDeath_Params
			{
				ATopBaseUnit* Unit = nullptr;
			};

			FEventUnitDeath_Params Params;
			Params.Unit = this;

			PC->ProcessEvent(Func, &Params);
		}
	}

	DeSelectUnit();
}

void ATopBaseUnit::BroadcastOnDeath()
{
	OnDeath.Broadcast();
}

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

void ATopBaseUnit::OnAttackTargetDeath_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("Attack target of %s has died"), *GetName());

	StopCommand();
}


#pragma endregion

#pragma region Commands

void ATopBaseUnit::StartAttackCommand_Implementation(ATopBaseUnit* attackTarget)
{
	SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
		EUnitMikroState::UnitMikroState_Attack_Rotate);

	InvalidateTarget();

	// Take in new target
	AttackTarget = attackTarget;

	// Hang onto its death event, in order to invalidate, scan for new targets eventually or fall back to idle
	attackTarget->OnDeath.AddDynamic(this, &ATopBaseUnit::OnAttackTargetDeath);
	
}

void ATopBaseUnit::StopCommand()
{
	InvalidateTarget();

	SetUnitState(EUnitMakroState::UnitMakroState_Idle, EUnitMikroState::UnitMikroState_Idle_Chilling);
}

void ATopBaseUnit::MoveToCommand_Implementation(const FVector &newPos)
{
	InvalidateTarget();

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

	//Now actually initiate moving task
	Task->ReadyForActivation();

}

#pragma endregion

#pragma region State_Machine

EUnitMakroState ATopBaseUnit::GetUnitMakroState() const
{
	return ECurrentUnitMakroState;
}

EUnitMikroState ATopBaseUnit::GetUnitMikroState() const
{
	return ECurrentUnitMikroState;
}


/*** You only should set two states with this at once, also inside the class*/
void ATopBaseUnit::SetUnitState(EUnitMakroState makroState, EUnitMikroState mikroState)
{
	if (ECurrentUnitMakroState != EUnitMakroState::UnitMakroState_Dead)
	{
		ECurrentUnitMakroState = makroState;
		ECurrentUnitMikroState = mikroState;

		p_fStateTimeCounter = 0.0f;

		STATE_LOG(Log, "SetUnitState() Makro: %s, Mikro %s",
			*UEnum::GetValueAsString(ECurrentUnitMakroState),
			*UEnum::GetValueAsString(ECurrentUnitMikroState));
	}
}


void ATopBaseUnit::IdleState(float DeltaSeconds)
{
	switch (ECurrentUnitMikroState) {
	case EUnitMikroState::UnitMikroState_Idle_Chilling:
		IdleChillingState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Idle_Cooldown:
		IdleCooldownState(DeltaSeconds);
		break;
	}
}

void ATopBaseUnit::IdleChillingState(float DeltaSeconds) {
	bool AttackModeSuccesful = TryAutoAttackIfTargetIsWithinMinimumRange();

	//TODO: Change this into a real AI Control for non humans player, that runs outside of state system
	if (!AttackModeSuccesful) {
		if (ACarnageGameState* GS = Cast<ACarnageGameState>(GetWorld()->GetGameState()))
		{
			if (UFactionState* FS = GS->GetFactionById(this->FactionId))
			{
				if (FS->ePlayerType == EPlayerType::Computer)
				{
					//In case the unit is computer controlled and it cant auto attack
					//it shall move close enough to the nearest enemy in order to be able to auto attack
					MoveToNearestEnemy();
				}
			}
		}
	}
}

void ATopBaseUnit::IdleCooldownState(float DeltaSeconds) {

}

void ATopBaseUnit::MovingState(float DeltaSeconds)
{
	// Access GameState and cast to ACarnageGameState
	if (UWorld* World = GetWorld())
	{
		if (ACarnageGameState* GS = World->GetGameState<ACarnageGameState>())
		{
			//Get Actor Locatoin
			FVector location = this->GetActorLocation();
			FVector2D Pos2D = FVector2D(location.X, location.Y);

			GS->mSpatialStorageManager->UpdateUnit(this, Pos2D);

		}
	}
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
	// If target is not valid -> back to idle
	if (!AttackTarget || !IsValid(AttackTarget))
	{
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
		return;
	}

	// Already facing target?
	if (IsFacingAttackTarget())
	{
		SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
			EUnitMikroState::UnitMikroState_Attack_Start);
	}
	else
	{
		// Rotate until facing
		RotateToAttackTarget(DeltaSeconds);
	}
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

#pragma region helpers

bool ATopBaseUnit::TryAutoAttackIfTargetIsWithinMinimumRange()
{
	// Query closest enemy
	const FClosestEnemyResult Closest = GetClosestEnemyUnit();

	// Validate enemy result
	if (!Closest.EnemyFound || !IsValid(Closest.ClosestEnemy))
	{
		return false;
	}

	// Check if within min range
	if (Closest.ClosestEnemyDistance < AttackComponent->MinRange)
	{
		// Start an attack command on this enemy
		StartAttackCommand(Closest.ClosestEnemy);
		return true;
	}

	// Out of range -> no attack started
	return false;
}

void ATopBaseUnit::InvalidateTarget()
{

	ATopBaseUnit* TargetUnit = Cast<ATopBaseUnit>(AttackTarget);

	// Remove delegate from old target about to be invalidated
	if (TargetUnit)
	{
		TargetUnit->OnDeath.RemoveDynamic(this, &ATopBaseUnit::OnAttackTargetDeath);
	}

	// Clear reference since target is gone
	AttackTarget = nullptr;

}

// --- Returns closest enemy unit, distance and a success flag ---
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

	// Build 2D position from our actor location 
	const FVector SelfLoc3D = GetActorLocation();
	const FVector2D SelfLoc2D(SelfLoc3D.X, SelfLoc3D.Y);

	// Ask the manager for the closest enemy actor using our faction
	AActor* FoundActor = GS->mSpatialStorageManager->GetClosestEnemyUnit(SelfLoc2D, FactionId);
	if (!FoundActor)
	{
		return Result;
	}

	if (ATopBaseUnit* FoundUnit = Cast<ATopBaseUnit>(FoundActor))
	{
		Result.ClosestEnemy = FoundUnit;

		const float Distance = FVector::Dist(FoundUnit->GetActorLocation(), SelfLoc3D);
		Result.ClosestEnemyDistance = Distance;

		Result.EnemyFound = true;
	}

	return Result;
}

/*** Manual collision detection that takes all units that are intruding our 
Capsule and pushes them out with a force. The deeper the intrusion the stronger
the force pushing out is, so take care while spawning units close to each other, unless you want them
to fly to the moon.*/
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

void ATopBaseUnit::MoveToNearestEnemy()
{
	AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
	if (!AI) { return; }

	const FClosestEnemyResult Closest = GetClosestEnemyUnit();
	if (!Closest.EnemyFound || !IsValid(Closest.ClosestEnemy)) { return; }

	// Stop any current movement before issuing a new request (mirrors BP)
	AI->StopMovement();

	// Enter moving state
	SetUnitState(EUnitMakroState::UnitMakroState_Moving,
		EUnitMikroState::UnitMikroState_Move_Direct_Move);

	// (Re)bind finish handler on the path following component
	if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
	{
		PF->OnRequestFinished.RemoveAll(this); // avoid duplicate bindings
		PF->OnRequestFinished.AddUObject(this, &ATopBaseUnit::OnMoveRequestFinished);
	}

	// Issue the move. This mirrors the BP pin setup: AcceptanceRadius=150, StopOnOverlap=false,
	// UsePathfinding=true, CanStrafe=false, AllowPartialPath=true.
	const EPathFollowingRequestResult::Type MoveRes =
		AI->MoveToActor(/*Goal*/ Closest.ClosestEnemy,
			/*AcceptanceRadius*/ 150.f,
			/*bStopOnOverlap*/   false,
			/*bUsePathfinding*/  true,
			/*bCanStrafe*/       false,
			/*FilterClass*/      nullptr,
			/*bAllowPartialPath*/true);

	// Immediate failure (no path etc.) -> mimic OnRequestFailed behavior
	if (MoveRes == EPathFollowingRequestResult::Failed)
	{
		AI->StopMovement();
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
		return;
	}
}

void ATopBaseUnit::OnMoveRequestFinished(FAIRequestID /*RequestID*/, const FPathFollowingResult& Result)
{
	// Always stop residual movement
	if (AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this))
	{
		if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
		{
			PF->OnRequestFinished.RemoveAll(this); // unbind to be safe
		}
		AI->StopMovement();
	}
	if (ECurrentUnitMakroState != EUnitMakroState::UnitMakroState_Dead) {
		// Return to idle (you can branch on Result.Code to chain into an attack on success)
// e.g. if (Result.Code == EPathFollowingResult::Success) { ... }
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
	}
}

bool ATopBaseUnit::IsFacingAttackTarget() const
{
	// Validate & cast AttackTarget to our own type
	ATopBaseUnit* TargetUnit = Cast<ATopBaseUnit>(AttackTarget);
	if (!TargetUnit || !IsValid(TargetUnit))
	{
		return false;
	}

	// World positions
	const FVector SelfLoc = GetActorLocation();
	const FVector TargetLoc = TargetUnit->GetActorLocation();

	// Desired look direction in XY
	const FVector2D Desired2D = FVector2D((TargetLoc - SelfLoc).GetSafeNormal2D());

	// Current forward direction in XY
	const FVector   Forward3D = GetActorForwardVector();
	const FVector2D Forward2D = FVector2D(Forward3D.X, Forward3D.Y).GetSafeNormal();

	// Dot-product threshold as in BP
	const double Dot = FVector2D::DotProduct(Desired2D, Forward2D);
	return Dot >= 0.995;
}

void ATopBaseUnit::RotateToAttackTarget(float DeltaTime)
{
	// Validate & cast attack target
	ATopBaseUnit* TargetUnit = Cast<ATopBaseUnit>(AttackTarget);
	if (!TargetUnit || !IsValid(TargetUnit))
	{
		return;
	}

	// Current & target positions
	const FVector SelfLoc = GetActorLocation();
	const FVector TargetLoc = TargetUnit->GetActorLocation();

	// Desired look-at rotation
	const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(SelfLoc, TargetLoc);

	// Current rotation
	const FRotator CurrentRot = GetActorRotation();

	// Interpolate smoothly (InterpSpeed = 10 as in BP)
	const FRotator NewRot = FMath::RInterpTo(CurrentRot, LookAtRot, DeltaTime, 10.f);

	// Apply to actor
	SetActorRotation(NewRot);
}





#pragma endregion

