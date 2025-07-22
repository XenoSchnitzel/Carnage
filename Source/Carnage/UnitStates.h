// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum UnitMakroState
{
	UnitMakroState_Idle				UMETA(DisplayName = "Idle"),
	UnitMakroState_Moving			UMETA(DisplayName = "Moving"),
	UnitMakroState_Attacking		UMETA(DisplayName = "Attacking"),
	UnitMakroState_Patrolling		UMETA(DisplayName = "Patrolling"),
	UnitMakroState_Pacifisting		UMETA(DisplayName = "Pacifisting"),
	UnitMakroState_Dead				UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum UnitMikroState
{
	UnitMikroState_Idle_Chilling				UMETA(DisplayName = "Idle_Chilling"),
	UnitMikroState_Idle_Cooldown				UMETA(DisplayName = "Idle_Cooldown"),

	UnitMikroState_Attack_Rotate				UMETA(DisplayName = "Attack_Rotate"),
	UnitMikroState_Attack_Start					UMETA(DisplayName = "Attack_Start"),
	UnitMikroState_Attack_Performing			UMETA(DisplayName = "Attack_Performing"),
	UnitMikroState_Attack_Cooldown_Start		UMETA(DisplayName = "Attack_Cooldown_Start"),
	UnitMikroState_Attack_Cooldown_Performing	UMETA(DisplayName = "Attack_Cooldown_Performing"),

	UnitMikroState_Move_Direct_Move				UMETA(DisplayName = "Move_Direct"),
	UnitMikroState_Move_Attack_Move				UMETA(DisplayName = "Move_Attack"),
	UnitMikroState_Move_To_Mining				UMETA(DisplayName = "Move_To_Mining"),
	UnitMikroState_Move_FromMining				UMETA(DisplayName = "Move_From_Mining")
};