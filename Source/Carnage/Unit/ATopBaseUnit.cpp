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

}
