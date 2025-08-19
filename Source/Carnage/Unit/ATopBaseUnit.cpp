// Copyright Epic Games, Inc. All Rights Reserved.
#include "ATopBaseUnit.h"
#include "../GameState/ACarnageGameState.h"

#include "../Logging/StateLogger.h"

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
		//checkf(false, TEXT("Unhandled MikroState in AttackingState: %s"),
		//	*UEnum::GetValueAsString(ECurrentUnitMikroState));
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

void ATopBaseUnit::Tick(float DeltaSeconds)
{
	p_fStateTimeCounter += DeltaSeconds;

	Super::Tick(DeltaSeconds);

	switch (ECurrentUnitMakroState) {
	case EUnitMakroState::UnitMakroState_Idle:
		IdleState(DeltaSeconds);
		break;
	case EUnitMakroState::UnitMakroState_Moving:
		MovingState(DeltaSeconds);
		break;
	case EUnitMakroState::UnitMakroState_Attacking:
		AttackingState(DeltaSeconds);
		break;
	default:
		//checkf(false, TEXT("Unhandled MikroState in AttackingState: %s"),
		//	*UEnum::GetValueAsString(ECurrentUnitMakroState));
		break;
	}
}

#pragma endregion