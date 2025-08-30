#include "MyHUD.h"
#include "Minimap.h"
#include "../PlayerController/CameraPawn.h"
#include "../GameState/ACarnageGameState.h"
#include "../Unit/ATopBaseUnit.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"

void AMyHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    ACameraPawn* CamPawn = Cast<ACameraPawn>(PC->GetPawn());
    if (!CamPawn) return;

    int32 SizeX, SizeY;
    PC->GetViewportSize(SizeX, SizeY);

    Minimap = NewObject<UMinimap>(this);
    if (Minimap)
    {
        Minimap->Initialize(CamPawn, FVector2D(SizeX, SizeY));
    }
}

void AMyHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Minimap) return;

    DrawMinimap();
    DrawUnits();
}

void AMyHUD::DrawMinimap()
{
    FMinimapFrameData Data = Minimap->GetFrameData();

    const float Size = 200.f;
    const float X = 50.f;
    const float Y = Canvas->SizeY - Size - 50.f;

    DrawRect(FLinearColor(0,0,0,0.5f), X, Y, Size, Size);

    if (Data.CameraFrustumPoints.Num() == 4)
    {
        for (int32 i = 0; i < 4; i++)
        {
            FVector2D P1 = Data.CameraFrustumPoints[i];
            FVector2D P2 = Data.CameraFrustumPoints[(i+1)%4];
            DrawLine(X+P1.X, Y+P1.Y, X+P2.X, Y+P2.Y, FLinearColor::Yellow, 1.f);
        }
    }
}

void AMyHUD::DrawUnits()
{
    ACarnageGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACarnageGameState>() : nullptr;
    if (!GS || !Minimap) return;

    // Hier später: Einheiten iterieren und WorldToMinimap() nutzen
    // z.B. DrawRect(FLinearColor::Green, Pos.X-1, Pos.Y-1, 2, 2);
}
