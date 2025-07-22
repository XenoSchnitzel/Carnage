//Copyright Epic Games, Inc. All Rights Reserved.

#include "ACarnageGameMode.h"
#include "../PlayerController/CarnagePlayerController.h"
#include "../PlayerController/CarnageCharacter.h"
#include "../PlayerController/CameraPawn.h"
#include "UObject/ConstructorHelpers.h"
#include "../GameState/ACarnageGameState.h"
#include "EReadyComponent.h"

ACarnageGameMode::ACarnageGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = ACarnagePlayerController::StaticClass();

	// //set default pawn class to our Blueprinted character
	//static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	//if (PlayerPawnBPClass.Class != nullptr)
	//{
	//	DefaultPawnClass = PlayerPawnBPClass.Class;
	//}
	DefaultPawnClass = ACameraPawn::StaticClass();


	GameStateClass = ACarnageGameState::StaticClass();

	//// set default controller to our Blueprinted controller
	//static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	//if(PlayerControllerBPClass.Class != NULL)
	//{
	//	PlayerControllerClass = PlayerControllerBPClass.Class;
	//}

}

void ACarnageGameMode::BeginPlay()
{
    Super::BeginPlay();
    ReadyComponents.Empty();
}

void ACarnageGameMode::RegisterReadyComponent(EReadyComponent Type, UObject* Sender)
{
    ReadyComponents.Add(Type);
    UE_LOG(LogTemp, Log, TEXT("Component registered: %s"), *UEnum::GetValueAsString(Type));
    TryFinalizeInitialization();
}

void ACarnageGameMode::TryFinalizeInitialization()
{
    if (ReadyComponents.Contains(EReadyComponent::GameState) &&
        ReadyComponents.Contains(EReadyComponent::HUD) /* && 
        ReadyComponents.Contains(EReadyComponent::AnotherImportantObject)*/)//Use this for further game logik that can only be wired when initalized
    {
        UE_LOG(LogTemp, Log, TEXT("All components ready, starting initialization"));

        ACarnageGameState* GS = GetGameState<ACarnageGameState>();

		AllCentralActorsRegistered.Broadcast();
    }
}