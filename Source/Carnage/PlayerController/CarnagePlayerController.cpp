//// Copyright Epic Games, Inc. All Rights Reserved.
//
#include "CarnagePlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "CarnageCharacter.h"
#include "Engine/World.h"

ACarnagePlayerController::ACarnagePlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ACarnagePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

//	if(bInputPressed)
//	{
//		FollowTime += DeltaTime;
//
//		// Look for the touch location
//		FVector HitLocation = FVector::ZeroVector;
//		FHitResult Hit;
//		if(bIsTouch)
//		{
//			GetHitResultUnderFinger(ETouchIndex::Touch1, ECC_Visibility, true, Hit);
//		}
//		else
//		{
//			GetHitResultUnderCursor(ECC_Visibility, true, Hit);
//		}
//		HitLocation = Hit.Location;
//
//		// Direct the Pawn towards that location
//		APawn* const MyPawn = GetPawn();
//		if(MyPawn)
//		{
//			FVector WorldDirection = (HitLocation - MyPawn->GetActorLocation()).GetSafeNormal();
//			MyPawn->AddMovementInput(WorldDirection, 1.f, false);
//		}
//	}
//	else
//	{
//		FollowTime = 0.f;
//	}
}

void ACarnagePlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	this->bEnableClickEvents = true;

	//InputComponent->BindAction("SetDestination", IE_Pressed, this, &ACarnagePlayerController::OnSetDestinationPressed);
	//InputComponent->BindAction("SetDestination", IE_Released, this, &ACarnagePlayerController::OnSetDestinationReleased);

	//// support touch devices 
	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &ACarnagePlayerController::OnTouchPressed);
	//InputComponent->BindTouch(EInputEvent::IE_Released, this, &ACarnagePlayerController::OnTouchReleased);
	//this->Possess(Cast<ACameraPawn>(ActorToPossess));
}

void ACarnagePlayerController::OnSetDestinationPressed()
{
	// We flag that the input is being pressed
	bInputPressed = true;
	// Just in case the character was moving because of a previous short press we stop it
	StopMovement();
}

void ACarnagePlayerController::OnSetDestinationReleased()
{
	// Player is no longer pressing the input
	bInputPressed = false;

	// If it was a short press
	if(FollowTime <= ShortPressThreshold)
	{
		// We look for the location in the world where the player has pressed the input
		FVector HitLocation = FVector::ZeroVector;
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, true, Hit);
		HitLocation = Hit.Location;

		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, HitLocation);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, HitLocation, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}

void ACarnagePlayerController::OnTouchPressed(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = true;
	OnSetDestinationPressed();
}

void ACarnagePlayerController::OnTouchReleased(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	bIsTouch = false;
	OnSetDestinationReleased();
}
