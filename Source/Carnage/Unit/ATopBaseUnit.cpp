// Copyright Epic Games, Inc. All Rights Reserved.
#include "ATopBaseUnit.h"
#include "../GameState/ACarnageGameState.h"

void ATopBaseUnit::BeginPlay()
{
	Super::BeginPlay();

	if (ACarnageGameState* GameState = Cast<ACarnageGameState>(GetWorld()->GetGameState()))
	{
		GameState->RegisterUnit(this);
	}

	this->ECurrentUnitMakroState = EUnitMakroState::UnitMakroState_Idle;
	this->ECurrentUnitMikroState = EUnitMikroState::UnitMikroState_Idle_Chilling;

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
}