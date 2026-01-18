// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "CameraPawn.generated.h"


UCLASS()
class CARNAGE_API ACameraPawn : public APawn
{
	GENERATED_BODY()

	bool bMouseWheelRotationActive = false;

	//Action Handlers
	void MiddleMouseButtonPressed();
	void MiddleMouseButtonReleased();
	void MouseWheelUp();
	void MouseWheelDown();
		

	void KeyRotateAroundCenter(float AxisValue);
	void MouseRotateAroundCenter(float Axisvalue);
	

	double CurrentVelocity;
	FRotator CurrentSpringArmRotation;

	FVector CurrentFrontalVel;
	FVector CurrentSideVel;
	double CurrentSpringArmRotationVelocity;

	TArray<float> ZoomLevels = { 1500.0f, 3000.0f, 4500.0f, 6000.0f, 7500.0f};
	int32 CurrentZoomIndex = 1; // Start bei 3000
	float ZoomInterpSpeed = 5.0f;
	float TargetZoomLength = 3000.0f;
	bool bIsMouseRotating = false;
	float MouseYawVelocity = 0.0f;

public:

	/** Moves the camera forward/backward (W/S, edge scroll Y) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
	void MoveFrontal(float AxisValue);

	/** Moves the camera left/right (A/D, edge scroll X) */
	UFUNCTION(BlueprintCallable, Category = "Camera|Movement")
	void MoveSideways(float AxisValue);

	// Sets default values for this pawn's properties
	ACameraPawn();

	/** Top down camera */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, category = Camera)
	class UCameraComponent* TopDownCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, category = Camera)
	class USpringArmComponent* CameraSpringArm;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
