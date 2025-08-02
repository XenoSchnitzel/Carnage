// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../GameState/enum/EFaction.h"
#include "../GameState/enum/EAlliance.h"


#include "ATopBaseUnit.generated.h"

//TODO: rename ATopBaseUnit to BaseUnit once full c++ migration has happened.
UCLASS(Blueprintable)
class ATopBaseUnit : public ACharacter
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		EFaction factionId;

	
};