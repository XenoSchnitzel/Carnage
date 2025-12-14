#include "ACarnageHUD.h"
#include "Minimap.h"
#include "../PlayerController/CameraPawn.h"
#include "../GameState/ACarnageGameState.h"
#include "../Unit/ATopBaseUnit.h"
#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"



void ACarnageHUD::BeginPlay()
{
    Super::BeginPlay();

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    ACameraPawn* CamPawn = Cast<ACameraPawn>(PC->GetPawn());
    if (!CamPawn) return;


    // Init verzögert aufrufen
    FTimerHandle InitHandle;
    GetWorld()->GetTimerManager().SetTimer(
        InitHandle, this, &ACarnageHUD::InitMinimap, 0.1f, false
    );
}

void ACarnageHUD::InitMinimap()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    int32 SizeX, SizeY;
    PC->GetViewportSize(SizeX, SizeY);
    if (SizeX == 0 || SizeY == 0) return; // not ready yet

    ACameraPawn* CamPawn = Cast<ACameraPawn>(PC->GetPawn());
    if (!CamPawn) return;

    Minimap = NewObject<UMinimap>(this);
    if (Minimap)
    {
        Minimap->Initialize(CamPawn, FVector2D(SizeX, SizeY));
    }
}

void ACarnageHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Minimap) return;

    DrawMinimap();
    DrawUnits();
}

static FVector2D RotateAround(
    const FVector2D& Point,
    const FVector2D& Pivot,
    float Degrees)
{
    const float Rad = FMath::DegreesToRadians(Degrees);
    const float S = FMath::Sin(Rad);
    const float C = FMath::Cos(Rad);

    FVector2D P = Point - Pivot;

    return FVector2D(
        P.X * C - P.Y * S,
        P.X * S + P.Y * C
    ) + Pivot;
}

void ACarnageHUD::DrawMinimap()
{
    FMinimapFrameData Data = Minimap->GetFrameData();

    FVector2D Size = Data.MiniMapFrameBox.GetSize();

    UTexture* MinimapTex = Data.renderTarget;

    FCanvasTileItem TileItem(
        FVector2D(Data.MiniMapFrameTopLeft.X, Data.MiniMapFrameTopLeft.Y),
        MinimapTex->GetResource(),
        Size,
        FLinearColor(1.f, 1.f, 1.f, 1.0f));

    // Rotation einstellen
    TileItem.Rotation = FRotator(0.f, -Data.MinimapRotation, 0.f);

    TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
    TileItem.BlendMode = SE_BLEND_Translucent;
    Canvas->DrawItem(TileItem);

    //Camera always points up and x,y are shifted in screen space, so rotate 90 degrees counterclockwise
    const float RotationDeg = -Data.MinimapRotation -90.0f;
    const FVector2D Pivot =
        Data.MiniMapFrameTopLeft + Data.MiniMapFrameBox.GetSize() * 0.5f;

    for (int32 i = 0; i < 4; i++)
    {
        FVector2D P1 = Data.CameraFrustumPoints[i];
        FVector2D P2 = Data.CameraFrustumPoints[(i + 1) % 4];

        P1 = RotateAround(P1, Pivot, RotationDeg);
        P2 = RotateAround(P2, Pivot, RotationDeg);

        DrawLine(P1.X, P1.Y, P2.X, P2.Y, FLinearColor::Yellow, 1.f);
    }
}

void ACarnageHUD::DrawUnits()
{
    ACarnageGameState* GS = GetWorld() ? GetWorld()->GetGameState<ACarnageGameState>() : nullptr;
    if (!GS || !Minimap) return;

    // Hier später: Einheiten iterieren und WorldToMinimap() nutzen
    // z.B. DrawRect(FLinearColor::Green, Pos.X-1, Pos.Y-1, 2, 2);
}
