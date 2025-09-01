// Fill out your copyright notice in the Description page of Project Settings.


#include "CameraPawn.h"

#define CARNAGE_CAMERA_VELOCITY				6000.0f
#define CARNAGE_CAMERA_DISTANCE				3000.0f
#define CARNAGE_CAMERA_KEY_ROTATION_SPEED	100.0f
#define CARNAGE_CAMERA_MOUSE_ROTATION_SPEED	300.0f


// Sets default values
ACameraPawn::ACameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera...
	this->CameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmTopDownCamera"));
	this->CameraSpringArm->SetupAttachment(RootComponent);
	//this->CameraSpringArm->SetUsingAbsoluteRotation(false); // Don't want arm to rotate when character does
	this->CameraSpringArm->TargetArmLength = CARNAGE_CAMERA_DISTANCE;
	this->CameraSpringArm->SetRelativeRotation(CurrentSpringArmRotation);
	this->CameraSpringArm->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level
	//this->CameraSpringArm->bUsePawnControlRotation = false;
	this->CameraSpringArm->bEnableCameraLag = true;

	// Create a camera...
	this->TopDownCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	this->TopDownCamera->SetupAttachment(CameraSpringArm, USpringArmComponent::SocketName);
	this->TopDownCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	AController* myController = APawn::GetController();
	if (myController)
	{
		CurrentSpringArmRotation = myController->GetControlRotation();
	}
	else
	{
		CurrentSpringArmRotation = FRotator();
	}

	CurrentSpringArmRotation.Pitch = -45.0;
	CurrentSpringArmRotationVelocity = 0.0f;
	this->CameraSpringArm->SetRelativeRotation(CurrentSpringArmRotation);
}

// Called when the game starts or when spawned
void ACameraPawn::BeginPlay()
{
	Super::BeginPlay();

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	AController* myController = APawn::GetController();
	if (myController)
	{
		CurrentSpringArmRotation = myController->GetControlRotation();
	}
	else
	{
		CurrentSpringArmRotation = FRotator();
	}

	CurrentSpringArmRotation.Pitch = -45.0;
	CurrentSpringArmRotationVelocity = 0.0f;
	this->CameraSpringArm->SetRelativeRotation(CurrentSpringArmRotation);

}

void ACameraPawn::MiddleMouseButtonPressed() {
	bMouseWheelRotationActive = true;
}

void ACameraPawn::MiddleMouseButtonReleased() {
	bMouseWheelRotationActive = false;
}

void ACameraPawn::MouseWheelUp() {
	if (CurrentZoomIndex > 0) {
		CurrentZoomIndex--;
		TargetZoomLength = ZoomLevels[CurrentZoomIndex];
		UE_LOG(LogTemp, Display, TEXT("Zoom in to %f"), TargetZoomLength);
	}
}

void ACameraPawn::MouseWheelDown() {
	if (CurrentZoomIndex < ZoomLevels.Num() - 1) {
		CurrentZoomIndex++;
		TargetZoomLength = ZoomLevels[CurrentZoomIndex];
		UE_LOG(LogTemp, Display, TEXT("Zoom out to %f"), TargetZoomLength);
	}
}

void ACameraPawn::MoveFrontal(float AxisValue)
{
	// Move forward or backward
	double frontalVelocity = FMath::Clamp(AxisValue, -1.0f, 1.0f) * CARNAGE_CAMERA_VELOCITY;
	
	//Get the normalized direction vector the camera is facing
	FVector forwardVector = GetActorForwardVector();

	forwardVector += forwardVector * frontalVelocity;

	//We only want to move above the terrain, not changing the camera height
	forwardVector.Z = 0.0;

	CurrentFrontalVel = forwardVector;
}

void ACameraPawn::MoveSideways(float AxisValue)
{
	// Move at 100 units per second forward or backward
	double sideVelocity = FMath::Clamp(AxisValue, -1.0f, 1.0f) * CARNAGE_CAMERA_VELOCITY;

	//Get the normalized direction vector the camera is facing
	FVector rightVector = GetActorRightVector();

	rightVector += rightVector * sideVelocity;

	//We only want to move above the terrain, not changing the camera height
	rightVector.Z = 0.0;

	CurrentSideVel = rightVector;
}

void ACameraPawn::KeyRotateAroundCenter(float AxisValue)
{
	// Move at 100 units per second forward or backward
	CurrentSpringArmRotationVelocity = FMath::Clamp(AxisValue, -1.0f, 1.0f) * CARNAGE_CAMERA_KEY_ROTATION_SPEED;
}

//void ACameraPawn::MouseRotateAroundCenter(float AxisValue)
//{
//	//////if (bMouseWheelRotationActive) {
//	//////	// Move at 100 units per second forward or backward
//	//////	CurrentSpringArmRotationVelocity = AxisValue * CARNAGE_CAMERA_MOUSE_ROTATION_SPEED;
//	//////}
//
//	//if (bMouseWheelRotationActive)
//	//{
//	//	// Statt direktem Setzen: sanftes Hinzufügen zur bestehenden Velocity
//	//	const float TargetVelocity = AxisValue * CARNAGE_CAMERA_MOUSE_ROTATION_SPEED;
//
//	//	// Smooth Interpolation zur Zielgeschwindigkeit
//	//	CurrentSpringArmRotationVelocity = FMath::FInterpTo(CurrentSpringArmRotationVelocity, TargetVelocity, GetWorld()->GetDeltaSeconds(), 30.0f);
//	//}
//	//else
//	//{
//	//	// Wenn nicht aktiv: sanft zurück zur Ruhe
//	//	CurrentSpringArmRotationVelocity = FMath::FInterpTo(CurrentSpringArmRotationVelocity, 0.0f, GetWorld()->GetDeltaSeconds(), 30.0f);
//	}
//}

// Called every frame
void ACameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (abs(CurrentSpringArmRotationVelocity) > 0)
	{
		CurrentSpringArmRotation.Yaw += CurrentSpringArmRotationVelocity * (double)DeltaTime;
		this->CameraSpringArm->SetRelativeRotation(FRotator(CurrentSpringArmRotation.Pitch,CurrentSpringArmRotation.Yaw,.0));
	}

	bool isMovingFrontal = !CurrentFrontalVel.IsZero();
	bool isMovingSideways = !CurrentSideVel.IsZero();

	if (isMovingFrontal || isMovingSideways) {
		
		FVector Location = GetActorLocation();
		
		if (isMovingFrontal) {
			// FPS independence
			FVector frontalMovementVector = CurrentFrontalVel * (double)DeltaTime;

			//Update the position
			Location += frontalMovementVector;
		}

		if (isMovingSideways) {			
			// FPS independence
			FVector sideMovementVector = CurrentSideVel * (double)DeltaTime;

			//Update the position
			Location += sideMovementVector;
		}

		SetActorLocation(Location);
	}

	// Smooth Zooming
	if (!FMath::IsNearlyEqual(CameraSpringArm->TargetArmLength, TargetZoomLength, 0.5f))
	{
		float NewLength = FMath::FInterpTo(CameraSpringArm->TargetArmLength, TargetZoomLength, DeltaTime, ZoomInterpSpeed);
		CameraSpringArm->TargetArmLength = NewLength;
	}

	//CameraSpringArm->TargetArmLength = TargetZoomLength;

	if (bMouseWheelRotationActive)
	{
		float MouseDeltaX;
		float MouseDeltaY;
		GetWorld()->GetFirstPlayerController()->GetInputMouseDelta(MouseDeltaX, MouseDeltaY);

		// Optional: Empfindlichkeit anpassen
		float TargetVelocity = MouseDeltaX * CARNAGE_CAMERA_MOUSE_ROTATION_SPEED;

		// Sanft interpolieren
		MouseYawVelocity = FMath::FInterpTo(MouseYawVelocity, TargetVelocity, DeltaTime, 10.0f);
	}
	else
	{
		// Wenn keine Rotation, sanft abbremsen
		MouseYawVelocity = FMath::FInterpTo(MouseYawVelocity, 0.0f, DeltaTime, 10.0f);
	}

	// Anwenden der Rotation (egal ob durch Taste oder Maus)
	if (!FMath::IsNearlyZero(CurrentSpringArmRotationVelocity) || !FMath::IsNearlyZero(MouseYawVelocity))
	{
		CurrentSpringArmRotation.Yaw += (CurrentSpringArmRotationVelocity + MouseYawVelocity) * DeltaTime;
		CameraSpringArm->SetRelativeRotation(FRotator(CurrentSpringArmRotation.Pitch, CurrentSpringArmRotation.Yaw, 0.0f));
	}
	
}

// Called to bind functionality to input
void ACameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set this pawn to be controlled by the lowest-numbered player
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Respond every frame to the values of our two movement axes, "MoveX" and "MoveY".
	InputComponent->BindAxis("FrontalMovement", this, &ACameraPawn::MoveFrontal);
	InputComponent->BindAxis("SideMovement", this, &ACameraPawn::MoveSideways);
	InputComponent->BindAxis("Rotation", this, &ACameraPawn::KeyRotateAroundCenter);
	//InputComponent->BindAxis("MouseRotation", this, &ACameraPawn::MouseRotateAroundCenter);

	// Bind an action to it
	InputComponent->BindAction
	(
		"MouseWheelDown", // The input identifier (specified in DefaultInput.ini)
		IE_Pressed, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ACameraPawn::MouseWheelDown // The function that will fire when input is received
	);

	// Bind an action to it
	InputComponent->BindAction
	(
		"MouseWheelUp", // The input identifier (specified in DefaultInput.ini)
		IE_Pressed, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ACameraPawn::MouseWheelUp // The function that will fire when input is received
	);

	InputComponent->BindAction
	(
		"MiddleMouseButton", // The input identifier (specified in DefaultInput.ini)
		IE_Released, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ACameraPawn::MiddleMouseButtonReleased // The function that will fire when input is received
	);

	// Bind an action to it
	InputComponent->BindAction
	(
		"MiddleMouseButton", // The input identifier (specified in DefaultInput.ini)
		IE_Pressed, // React when button pressed (or on release, etc., if desired)
		this, // The object instance that is going to react to the input
		&ACameraPawn::MiddleMouseButtonPressed // The function that will fire when input is received
	);
}

