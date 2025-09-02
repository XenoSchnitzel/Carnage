#include "ATopBaseUnit.h"
#include "../GameState/ACarnageGameState.h"

#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "AIController.h"
#include "Tasks/AITask_MoveTo.h"
#include "AITypes.h"  // for FAIRequestID
#include "NiagaraFunctionLibrary.h"   // für UNiagaraFunctionLibrary::SpawnSystemAtLocation
#include "NiagaraSystem.h"            // für UNiagaraSystem (Asset-Klasse)
#include "NavigationSystem.h"

#include "../Decals/DecalManager.h"
#include "../Decals/DecalLibrary.h"

#include "Navigation/PathFollowingComponent.h"   // for EPath

#include "../Logging/StateLogger.h"
#include "../SpatialStorage/RTSUnitManagerComponent.h"

#include <Carnage\Resources\AResourceNode.h>

#include "EUnitStates.h"


FVector GetNavMeshTargetNearActor(AActor* Building, AActor* Unit, float Buffer, UWorld* World)
{
	if (!Building || !Unit || !World)
	{
		return FVector::ZeroVector;
	}

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
	if (!NavSys)
	{
		return FVector::ZeroVector;
	}

	// Bounding Box des Gebäudes
	FBox Bounds = Building->GetComponentsBoundingBox();
	FVector Center = Bounds.GetCenter();

	// Richtung von der Unit zum Gebäudezentrum
	FVector Dir = (Center - Unit->GetActorLocation()).GetSafeNormal();

	// Nächstgelegener Punkt der Bounds zur Unit
	FVector ClosestPoint = Bounds.GetClosestPointTo(Unit->GetActorLocation());

	// Einen Buffer weiter nach außen schieben
	FVector Target = ClosestPoint + Dir * Buffer;

	// Auf NavMesh projizieren
	FNavLocation ProjectedLoc;
	if (NavSys->ProjectPointToNavigation(Target, ProjectedLoc, FVector(500, 500, 500)))
	{
		return ProjectedLoc.Location;
	}

	// Falls das fehlschlägt → als Fallback das nächstgelegene NavMesh im Umkreis des Gebäudes nehmen
	if (NavSys->ProjectPointToNavigation(Center, ProjectedLoc, FVector(2000, 2000, 2000)))
	{
		return ProjectedLoc.Location;
	}

	// Nichts gefunden
	return FVector::ZeroVector;
}


#pragma region Construction

ATopBaseUnit::ATopBaseUnit()
{
	AttackComponent = CreateDefaultSubobject<UAttackComponent>(TEXT("AttackComponent"));
	HitpointComponent = CreateDefaultSubobject<UHitpointComponent>(TEXT("HitpointComponent"));
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

#pragma endregion

#pragma region events

void ATopBaseUnit::ShowAttackIndicator_Implementation(bool show) {}

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

		SetActorEnableCollision(true);


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
	STATE_LOG(this, Log, "Attack target of %s has died", *GetName());


	StopCommand();
}


#pragma endregion

#pragma region Commands

void ATopBaseUnit::StartAttackCommand_Implementation(ATopBaseUnit* attackTarget)
{
	SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
		EUnitMikroState::UnitMikroState_Attack_Rotate);

	InvalidateAttackTarget();

	// Take in new target
	AttackTarget = attackTarget;

	// Hang onto its death event, in order to invalidate, scan for new targets eventually or fall back to idle
	attackTarget->OnDeath.AddDynamic(this, &ATopBaseUnit::OnAttackTargetDeath);
	
}

void ATopBaseUnit::StopCommand()
{
	SetUnitState(EUnitMakroState::UnitMakroState_Idle, EUnitMikroState::UnitMikroState_Idle_Chilling);
}

void ATopBaseUnit::MiningResourceCommand_Implementation(AResourceNode* miningNode) {

	SetUnitState(EUnitMakroState::UnitMakroState_Mining,
		EUnitMikroState::UnitMikroState_Mining_Move_To_Resource);

	InvalidateMiningTarget();

	MiningTarget = miningNode;

	AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
	if (!AI) { return; }

	// (Re)bind finish handler on the path following component
	if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
	{
		PF->OnRequestFinished.RemoveAll(this); // avoid duplicate bindings
		PF->OnRequestFinished.AddUObject(this, &ATopBaseUnit::OnMoveRequestFinished);
	}

	// Issue the move. This mirrors the BP pin setup: AcceptanceRadius=150, StopOnOverlap=false,
	// UsePathfinding=true, CanStrafe=false, AllowPartialPath=true.
	const EPathFollowingRequestResult::Type MoveRes =
		AI->MoveToActor(/*Goal*/ miningNode,
			/*AcceptanceRadius*/ 150.f,
			/*bStopOnOverlap*/   false,
			/*bUsePathfinding*/  true,
			/*bCanStrafe*/       false,
			/*FilterClass*/      nullptr,
			/*bAllowPartialPath*/true);

	// Immediate failure (no path etc.) -> mimic OnRequestFailed behavior
	if (MoveRes == EPathFollowingRequestResult::Failed)
	{
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
		return;
	}
}

void ATopBaseUnit::MoveToCommand_Implementation(const FVector& NewPos)
{
	// Update state
	SetUnitState(
		EUnitMakroState::UnitMakroState_Moving,
		EUnitMikroState::UnitMikroState_Move_Direct_Move);

	// Grab AIController
	AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
	if (!AI) { return; }

	// (Re)bind handler on PathFollowingComponent
	if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
	{
		PF->OnRequestFinished.RemoveAll(this); // prevent duplicates
		PF->OnRequestFinished.AddUObject(this, &ATopBaseUnit::OnMoveRequestFinished);
	}

	// Issue the move (like AIMoveTo, but directly through AIController)
	const EPathFollowingRequestResult::Type MoveRes =
		AI->MoveToLocation(
			/*GoalLocation*/ NewPos,
			/*AcceptanceRadius*/ 50.f,
			/*bStopOnOverlap*/ true,
			/*bUsePathfinding*/ true,
			/*bProjectDestination*/ true,
			/*bcanStrafe*/ true,
			/*FilterClass*/ nullptr,
			/*bAllowPartialPath*/ false);

	// Immediate failure handling
	if (MoveRes == EPathFollowingRequestResult::Failed)
	{
		SetUnitState(
			EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
	}
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
void ATopBaseUnit::SetUnitState(EUnitMakroState newMakroState, EUnitMikroState newMikroState)
{
	//Dependant on the unit state we do certain cleanups, so we dont have
	//to think about them from the outside in any circumstance

	switch (ECurrentUnitMakroState)
	{
		case EUnitMakroState::UnitMakroState_Moving:
		{
			AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
			if (!AI) break;
			// If you don't terminate an ongoing movement, you
			// run into trouble with the AI trying to reach
			// all instructed positions
			const EPathFollowingStatus::Type MoveStatus = AI->GetMoveStatus();
			if (MoveStatus == EPathFollowingStatus::Moving)
			{
				AI->StopMovement();
			}
			break;
		}

		case EUnitMakroState::UnitMakroState_Attacking:
		{
			if (newMakroState != EUnitMakroState::UnitMakroState_Attacking)
			{
				//Get rid of my attack target when NOT attacking anymore
				InvalidateAttackTarget();

				this->ShowAttackIndicator(false);
			}
			break;
		}

		case EUnitMakroState::UnitMakroState_Mining:

			if (newMakroState != EUnitMakroState::UnitMakroState_Mining) {
				InvalidateMiningTarget();
			}
			break;

		case EUnitMakroState::UnitMakroState_Dead:
			return;

		default:
			break;
	}

	ECurrentUnitMakroState = newMakroState;
	ECurrentUnitMikroState = newMikroState;

	p_fStateTimeCounter = 0.0f;

	STATE_LOG(this, Log, "SetUnitState() Makro: %s, Mikro %s",
		*UEnum::GetValueAsString(ECurrentUnitMakroState),
		*UEnum::GetValueAsString(ECurrentUnitMikroState));	
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
	if (p_fStateTimeCounter >= AttackComponent->CoolDownTime) {
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
	}
}

void ATopBaseUnit::MovingState(float DeltaSeconds)
{
	// Access GameState and cast to ACarnageGameState
	if (UWorld* World = GetWorld())
	{
		if (ACarnageGameState* GS = World->GetGameState<ACarnageGameState>())
		{
			//Get Actor Location
			FVector location = this->GetActorLocation();
			FVector2D Pos2D = FVector2D(location.X, location.Y);

			GS->mSpatialStorageManager->UpdateUnit(this, Pos2D);

		}
	}
}


void ATopBaseUnit::MiningState(float DeltaSeconds)
{
	switch (ECurrentUnitMikroState) {
	case EUnitMikroState::UnitMikroState_Mining_Move_To_Resource:
		MiningMoveToState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Mining_Move_From_Resource:
		MiningMoveFromState(DeltaSeconds);
		break;
	case EUnitMikroState::UnitMikroState_Mining_At_Resource:
		MiningAtState(DeltaSeconds);
		break;
	default:
		checkf(false, TEXT("Unhandled MikroState in MiningState: %s"),
			*UEnum::GetValueAsString(ECurrentUnitMikroState));
		break;
	}
}

void ATopBaseUnit::MiningMoveToState(float DeltaSeconds)
{

}

void ATopBaseUnit::MiningMoveFromState(float DeltaSeconds)
{

}

void ATopBaseUnit::MiningAtState(float DeltaSeconds)
{
	//TODO: Change into Mining Component or something
	if (p_fStateTimeCounter >= 1.0f) {

		//TODO: Take money from crystal $$$

	//	ACarnageGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACarnageGameState>() : nullptr;

	//	AActor* baseBuilding = GS->GetFactionById(this->FactionId)->GetMainBaseBuilding();

	//	AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this);
	//	if (!AI) { return; }


	//	// Enter moving state
	//	SetUnitState(EUnitMakroState::UnitMakroState_Mining,
	//		EUnitMikroState::UnitMikroState_Move_FromMining);

	//	// (Re)bind finish handler on the path following component
	//	if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
	//	{
	//		PF->OnRequestFinished.RemoveAll(this); // avoid duplicate bindings
	//		PF->OnRequestFinished.AddUObject(this, &ATopBaseUnit::OnMoveRequestFinished);
	//	}

	//	FVector TargetLoc = GetNavMeshTargetNearActor(baseBuilding, this, 200.f, GetWorld());


	//	/*UE_LOG(LogTemp, Warning, TEXT("BaseBuilding Location: %s"), *baseBuilding->GetActorLocation().ToString());
	//	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	//	FNavLocation OutLoc;
	//	bool bOnNav = NavSys->ProjectPointToNavigation(baseBuilding->GetActorLocation(), OutLoc);
	//	UE_LOG(LogTemp, Warning, TEXT("Projected to NavMesh: %d -> %s"), bOnNav, *OutLoc.Location.ToString());*/

	//	// Issue the move. This mirrors the BP pin setup: AcceptanceRadius=150, StopOnOverlap=false,
	//	// UsePathfinding=true, CanStrafe=false, AllowPartialPath=true.
	//// Issue the move (like AIMoveTo, but directly through AIController)

	//	if (!TargetLoc.IsZero())
	//	{
	//		const EPathFollowingRequestResult::Type MoveRes =
	//			AI->MoveToLocation(
	//				/*GoalLocation*/ TargetLoc,
	//				/*AcceptanceRadius*/ 150.f,
	//				/*bStopOnOverlap*/ true,
	//				/*bUsePathfinding*/ true,
	//				/*bProjectDestination*/ true,
	//				/*bcanStrafe*/ true,
	//				/*FilterClass*/ nullptr,
	//				/*bAllowPartialPath*/ false);

	//		// Immediate failure (no path etc.) -> mimic OnRequestFailed behavior
	//		if (MoveRes == EPathFollowingRequestResult::Failed)
	//		{
	//			SetUnitState(EUnitMakroState::UnitMakroState_Idle,
	//				EUnitMikroState::UnitMikroState_Idle_Chilling);
	//			return;
	//		}
	//	}
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
	if (!AttackTarget || !IsValid(AttackTarget)) {
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
		return;
	}

	// Already facing target?
	if (IsFacingAttackTarget()) {
		SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
			EUnitMikroState::UnitMikroState_Attack_Start);
	}
	else {
		// Rotate until facing
		RotateToAttackTarget(DeltaSeconds);
	}
}

void ATopBaseUnit::AttackStartState(float DeltaSeconds)
{
	if (TryAttackTarget()) {
		this->ShowAttackIndicator(true);

		SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
			EUnitMikroState::UnitMikroState_Attack_Performing);
	}
	else {
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Cooldown);
	}
}

void ATopBaseUnit::AttackPerfomingState(float DeltaSeconds)
{
	if (p_fStateTimeCounter >= AttackComponent->AttackTime) {
		SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
			EUnitMikroState::UnitMikroState_Attack_Cooldown_Start);
	}
}

void ATopBaseUnit::AttackCooldownStartState(float DeltaSeconds)
{
	this->ShowAttackIndicator(false);
	
	SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
		EUnitMikroState::UnitMikroState_Attack_Cooldown_Performing);
}

void ATopBaseUnit::AttackCooldownPeformingState(float DeltaSeconds)
{
	if (p_fStateTimeCounter >= AttackComponent->CoolDownTime) {
		SetUnitState(EUnitMakroState::UnitMakroState_Attacking,
			EUnitMikroState::UnitMikroState_Attack_Rotate);
	}
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
		case EUnitMakroState::UnitMakroState_Mining:
			MiningState(DeltaTime);
			break;
		default:
			//checkf(false, TEXT("Unhandled MikroState in AttackingState: %s"),
			//	*UEnum::GetValueAsString(ECurrentUnitMakroState));
			break;
	}
}

#pragma endregion

#pragma region helpers

bool ATopBaseUnit::TryAttackTarget() {

	//1. First we try calculate:
	//   * a start vector (e.g. shooting/slashing/... start point) coming from the attacking actor
	//   * an attack vector pointing towards the target
	UCapsuleComponent* capsule = GetCapsuleComponent();

	const FVector attackerOrigin = GetActorLocation();
	FVector fwdVec = GetActorForwardVector();

	//Enlarge the forward vector with a little additional buffer 
	//so it is definetely outside the capsule
	fwdVec *= (capsule->GetScaledCapsuleRadius() + 10.0f);

	//We add half the capsule height for the actor beeing able to shoot from
	//heighened positions without touching the ground that easily
	const FVector attackStartVectorPoint = attackerOrigin + FVector(fwdVec.X, fwdVec.Y, capsule->GetScaledCapsuleHalfHeight());

	//Calculate the vector from the attacker to the attacked 
	FVector attackVector = AttackTarget->GetActorLocation() - attackerOrigin;

	//2. Random values are calculated to determine a "shooting cone" of possible values, 
	//   dependent on distance and a unit specific spread value.
	//   With this we can determine the end point vector of the attack.

	//Dependant on the length a spread value is calulated in
	float spreadValue = attackVector.Length() * AttackComponent->Spread;

	//Now we calculate three random components based on the spreadvalue for each dimension 
	float xSpreadValue = FMath::FRandRange(-spreadValue, spreadValue);
	float ySpreadValue = FMath::FRandRange(-spreadValue, spreadValue);
	float zSpreadValue = FMath::FRandRange(-spreadValue, spreadValue);

	const FVector attackEndVectorPoint = 
		attackStartVectorPoint + 
		attackVector * 2.0f + // double the attack vector it so it is definetely long enough to penetrate the target
		FVector(xSpreadValue, ySpreadValue, zSpreadValue);

	//3. Now we make a line trace in order to check wether the target was hit

	FHitResult HitResult;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);   // entspricht "Ignore Self = true"

	// wähle deinen Kanal (im BP war es TraceTypeQuery1)
	const ETraceTypeQuery TraceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);

	bool bTraceComplex = false;

	//Meelee attacks can "penetrate" other units, 
	// while distance attacks through friendly units are
	// blocked. (Design decision)
	// 
	// Mathematics for melee would be too complex in close combat with many actors
	if(AttackComponent->Type == EAttackType::Melee) {
		bTraceComplex = true;
	}

	// Debug Draw (optional)
	EDrawDebugTrace::Type DebugDraw = EDrawDebugTrace::None;

	//4. BOOOOM
	bool bHit = UKismetSystemLibrary::LineTraceSingle(
		GetWorld(),
		attackStartVectorPoint,
		attackEndVectorPoint,
		TraceChannel,
		bTraceComplex,
		ActorsToIgnore,
		DebugDraw,
		HitResult,
		true,          // bIgnoreSelf
		FLinearColor::Red,
		FLinearColor::Green,
		0.2f           // DrawTime
	);

	//5. In case we hit something, we have to differentiate what we hit

	//if (GEngine)
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("=== Listing Worlds ==="));
	//	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	//	{
	//		if (UWorld* World = Context.World())
	//		{
	//			UE_LOG(LogTemp, Warning, TEXT("World: %s  Type=%s  PIE=%d"),
	//				*World->GetName(),
	//				*UEnum::GetValueAsString(Context.WorldType),
	//				(int)Context.PIEInstance);
	//		}
	//	}
	//}

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		if (auto* HitUnit = Cast<ATopBaseUnit>(HitResult.GetActor()))
		{
			//We hit a unit

			if (this->FactionId != HitUnit->FactionId) // dein Vergleich aus dem BP
			{
				//We hit an enemy unit

				//Spawn a blood niagara system at the hitpoint
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = this;
				SpawnParams.Instigator = GetInstigator();

				UClass* BloodSpitClass = LoadClass<AActor>(
					nullptr,
					TEXT("/Game/TopDown/Blueprints/Units/BP_Actor_BloodSpit.BP_Actor_BloodSpit_C")
				);

				if (!BloodSpitClass)
				{
					UE_LOG(LogTemp, Error, TEXT("LoadClass failed for BP_Actor_BloodSpit"));
				}
				else
				{
					GetWorld()->SpawnActor<AActor>(
						BloodSpitClass,
						HitResult.ImpactPoint,
						HitResult.ImpactNormal.Rotation()
					);
				}

				//Notify the other unit to make damage calculation
				HitUnit->OnHit(this);

				//This attack impulse is currently calculated in only after death, when physics enabled for ragdoll handling
				// however the shot, that brings a unit to death should already be considered.
				if (USkeletalMeshComponent* LeChuck = HitUnit->FindComponentByClass<USkeletalMeshComponent>())
				{
					FVector Impulse = attackVector.GetSafeNormal() * 300.0f;
					LeChuck->AddImpulseToAllBodiesBelow(Impulse, NAME_None, true);
				}


				return true; //Successful Hit
			}
			else
			{
				//we hit a friendly unit, no friendly fire at the moment
				//We currently dont have any attack markings for this,
				//however we may need in the future, for instance artillery attacks with splash damage, etc.
			}
		}
		else
		{
			//We hit a "non unit" actor, like a stone, landscape, something moveable but not attributed to any team...

			//Spawn a hit marker
			if (auto* DM = GetWorld()->GetSubsystem<UCarnageDecalManager>())
			{
				DM->SpawnDecalByTagAtHit(HitResult, FGameplayTag::RequestGameplayTag(FName("Decal.HitMetal")));
			}
		}

		// fallback branch, hitting the air currently has no effect so no else branch here
	}

	return false;
}

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

void ATopBaseUnit::InvalidateAttackTarget()
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

void ATopBaseUnit::InvalidateMiningTarget()
{

	//TODO: Handle coppling of "mined out" events

	// Clear reference since target is gone
	MiningTarget = nullptr;

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
		SetUnitState(EUnitMakroState::UnitMakroState_Idle,
			EUnitMikroState::UnitMikroState_Idle_Chilling);
		return;
	}
}

void ATopBaseUnit::OnMoveRequestFinished(FAIRequestID /*RequestID*/, const FPathFollowingResult& Result)
{
	// Clean up
	if (AAIController* AI = UAIBlueprintHelperLibrary::GetAIController(this))
	{
		if (UPathFollowingComponent* PF = AI->GetPathFollowingComponent())
		{
			PF->OnRequestFinished.RemoveAll(this);
		}
		AI->StopMovement();
	}

	// Evaluate result
	if (Result.IsSuccess())
	{
		STATE_LOG(this, Log, "Move success");

		if (ECurrentUnitMakroState == EUnitMakroState::UnitMakroState_Mining) {

			if (ECurrentUnitMikroState == EUnitMikroState::UnitMikroState_Mining_Move_From_Resource) {
				
				//TODO: Add Money to bank $$$

				MiningResourceCommand_Implementation(MiningTarget);

			}
			else { //Mining To Resource state
				SetUnitState(EUnitMakroState::UnitMakroState_Mining,
					EUnitMikroState::UnitMikroState_Mining_At_Resource);
			}
		}
		else {
			SetUnitState(EUnitMakroState::UnitMakroState_Idle,
				EUnitMikroState::UnitMikroState_Idle_Chilling);
		}
	}
	else
	{
		STATE_LOG(this, Log, "Move failed or aborted (%i)", (int)Result.Code);
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

