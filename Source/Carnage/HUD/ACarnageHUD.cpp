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

FUIHitInfo ACarnageHUD::CheckIfUIisHit(int screen_x, int screen_y)
{
    //Currently we only have this case, but other custom UI Elements may have to handled in the future...
    return this->Minimap->CheckIfMinimapIsHit(screen_x, screen_y);
}

void ACarnageHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Minimap) return;

    DrawMinimap();
    DrawUnits();
}

void ACarnageHUD::DrawMinimap()
{
    //Would have been handled better funtionality-encapsulation-wise aa method of the minimap object.
    //Since the Canvas and drawing functions are part of the HUD Object, transfering this to the outside
    //seems to be kind of delicate, so I currently prefer this solution:

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

    for (int32 i = 0; i < 4; i++)
    {
        FVector2D P1 = Data.CameraFrustumPoints[i];
        FVector2D P2 = Data.CameraFrustumPoints[(i + 1) % 4];

        DrawLine(P1.X, P1.Y, P2.X, P2.Y, FLinearColor::Yellow, 1.f);

        P1 = Data.MiniMapFramePoints[i];
        P2 = Data.MiniMapFramePoints[(i + 1) % 4];

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
