// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SelectionUtilities.generated.h"

USTRUCT(BlueprintType)
struct FUSelectionUtilitiesReturnType
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		double length;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FVector newPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		AActor* selectedUnit;

	FUSelectionUtilitiesReturnType()
	{

	}

};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CARNAGE_API USelectionUtilities : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USelectionUtilities();

protected:

	FVector lastDirection;

	// Called when the game starts
	virtual void BeginPlay() override;

	//TArray<FVector> MatchToClosestPositions(const TArray<AActor*>& Actors, const TArray<FVector>& TargetPositions);
	TArray<FUSelectionUtilitiesReturnType> MatchToClosestPositions(const TArray<AActor*>& Actors, const TArray<FVector>& TargetPositions, const FVector& CurrentForwardDirection);

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	TArray<FUSelectionUtilitiesReturnType> MoveSelectionToLocation(FVector location, TArray<AActor*> selectionUnits);


	//UFUNCTION(BlueprintCallable)
    //TArray<FUSelectionUtilitiesReturnType> ArrangeActorsInLineFormation(const TArray<AActor*>& Actors, const FVector& CenterLocation, FVector ForwardDirection, float Strength);

	UFUNCTION(BlueprintCallable)
	TArray<FUSelectionUtilitiesReturnType> ArrangeActorsInLineFormation(const TArray<AActor*>& Actors, const FVector& CenterLocation, FVector ForwardDirection, float Strength);
		
	//void MoveSelectionToLocation(FVector location);
};
