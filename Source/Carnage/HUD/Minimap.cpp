#include "Minimap.h"
#include "../PlayerController/CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"

UMinimap::UMinimap()
{
    ConstructorHelpers::FObjectFinder<UTexture> MapTexObj(TEXT("/Game/TopDown/Sprites/TopView_Texture.TopView_Texture"));
    if (MapTexObj.Succeeded())
    {
        MapTex = MapTexObj.Object;
    }
}

void UMinimap::Initialize(ACameraPawn* InCamera, const FVector2D& InScreenSize, const FBox2D& InWorldBounds)
{
    CameraPawn = InCamera;
    ScreenSize = InScreenSize;

    this->MiniMapFrameTopLeft = FVector2D(CARNAGE_MINIMAP_X, CARNAGE_MINIMAP_Y);
    FVector2D MiniMapFrameBottomRight = FVector2D(CARNAGE_MINIMAP_X + CARNAGE_MINIMAP_SIZE, CARNAGE_MINIMAP_Y + CARNAGE_MINIMAP_SIZE);
    this->MiniMapFrameBox = FBox2D(this->MiniMapFrameTopLeft, MiniMapFrameBottomRight);

    this->RenderTarget = UCanvasRenderTarget2D::CreateCanvasRenderTarget2D(
        GetWorld(), UCanvasRenderTarget2D::StaticClass(), CARNAGE_MINIMAP_SIZE, CARNAGE_MINIMAP_SIZE);

    RenderTarget->ClearColor = FLinearColor(0, 0, 0, 0); // fully transparent

    // Delegate binden
    RenderTarget->OnCanvasRenderTargetUpdate.AddDynamic(this, &UMinimap::DrawMinimapToTexture);

    if (InWorldBounds.bIsValid)
    {
        WorldBounds = InWorldBounds;
    }
    else
    {
        DetectWorldBoundsFromLandscape();
    }

    // Minimap rectangle (always square)
    const FVector2D MinimapOrigin(0.0f, 0.0f);
    const FVector2D MinimapSize(CARNAGE_MINIMAP_SIZE, CARNAGE_MINIMAP_SIZE);

    // World size
    const FVector2D WorldSize = WorldBounds.GetSize();
    const float WorldAspect = WorldSize.X / WorldSize.Y;
    const float MinimapAspect = 1.0f; // square

    // Resulting sub-rectangle inside the minimap
    FVector2D SubSize;
    FVector2D SubOrigin = MinimapOrigin;

    if (WorldAspect > MinimapAspect)
    {
        // World is wider → scale to full width, reduce height
        SubSize.X = MinimapSize.X;
        SubSize.Y = MinimapSize.Y / WorldAspect;
        SubOrigin.Y += (MinimapSize.Y - SubSize.Y) * 0.5f; // center vertically
    }
    else
    {
        // World is taller → scale to full height, reduce width
        SubSize.Y = MinimapSize.Y;
        SubSize.X = MinimapSize.X * WorldAspect;
        SubOrigin.X += (MinimapSize.X - SubSize.X) * 0.5f; // center horizontally
    }

    // Store results
    MiniMapTopLeft = SubOrigin;
    MiniMapBottomRight = SubOrigin + FVector2D(SubSize.X, SubSize.Y);

    UpdateFrameData();
}

void UMinimap::Tick(float DeltaSeconds)
{
    if (bDragging)
    {
        MoveCameraTo(DragTarget);
    }
    UpdateFrameData();
}

void UMinimap::UpdateFrameData()
{
    if (!CameraPawn) return;

    CurrentFrameData.renderTarget = this->RenderTarget;
    CurrentFrameData.MiniMapFrameTopLeft = this->MiniMapFrameTopLeft;
    CurrentFrameData.MiniMapFrameBox = this->MiniMapFrameBox;
    CurrentFrameData.MinimapRotation = CameraPawn->GetActorRotation().Yaw;
    CurrentFrameData.MinimapCenter   = FVector2D(
        CARNAGE_MINIMAP_SIZE / 2.0f,
        CARNAGE_MINIMAP_SIZE / 2.0f);

    CurrentFrameData.CameraFrustumPoints = ComputeCameraFrustum();
    CurrentFrameData.UnitPositions.Empty();

    // Aktualisieren
    RenderTarget->UpdateResource(); // löst das Zeichnen ins Target aus
}

void UMinimap::DetectWorldBoundsFromLandscape()
{
    WorldBounds.Init();
    if (!CameraPawn) return;

    UWorld* World = CameraPawn->GetWorld();
    if (!World) return;

    for (TActorIterator<ALandscape> It(World); It; ++It)
    {
        FBox Bounds = It->GetComponentsBoundingBox();
        WorldBounds = FBox2D(FVector2D(Bounds.Min.X, Bounds.Min.Y),
                             FVector2D(Bounds.Max.X, Bounds.Max.Y));

        UE_LOG(LogTemp, Display, TEXT("Minimap: Landscape bounds detected Min(%.1f, %.1f) Max(%.1f, %.1f)"),
            WorldBounds.Min.X, WorldBounds.Min.Y,
            WorldBounds.Max.X, WorldBounds.Max.Y);
        break;
    }

    if (!WorldBounds.bIsValid)
    {
        WorldBounds = FBox2D(FVector2D(-5000, -5000), FVector2D(5000, 5000));
        UE_LOG(LogTemp, Warning, TEXT("Minimap: No Landscape found – using fallback bounds"));
    }
}

FVector2D UMinimap::WorldToMinimap(const FVector& WorldPos) const
{
    // 1) Normalisieren in WorldBounds
    const FVector2D WorldSize = WorldBounds.GetSize();
    FVector2D Norm(
        (WorldPos.X - WorldBounds.Min.X) / WorldSize.X,
        (WorldPos.Y - WorldBounds.Min.Y) / WorldSize.Y
    );

    // 2) In den "World-Subbereich" innerhalb der Minimap-Texture (0..400)
    const FVector2D SubOrigin = MiniMapTopLeft;
    const FVector2D SubSize = MiniMapBottomRight - MiniMapTopLeft;
    FVector2D LocalPos = SubOrigin + FVector2D(Norm.X * SubSize.X,
        Norm.Y * SubSize.Y);

    // 3) Von Texture-Space in Screen-Space (Frame-Offset)
    return MiniMapFrameTopLeft + LocalPos;
}

FVector UMinimap::MinimapToWorld(const FVector2D& MapPos) const
{
    FVector2D Norm(MapPos.X / ScreenSize.X, MapPos.Y / ScreenSize.Y);
    return FVector(
        FMath::Lerp(WorldBounds.Min.X, WorldBounds.Max.X, Norm.X),
        FMath::Lerp(WorldBounds.Min.Y, WorldBounds.Max.Y, Norm.Y),
        0.f);
}

TArray<FVector2D> UMinimap::ComputeCameraFrustum() const
{
    TArray<FVector2D> Out;
    if (!CameraPawn || !CameraPawn->TopDownCamera) return Out;

    UWorld* World = CameraPawn->GetWorld();
    if (!World) return Out;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return Out;

    FSceneViewProjectionData ProjectionData;
    PC->GetLocalPlayer()->GetProjectionData(PC->GetLocalPlayer()->ViewportClient->Viewport, ProjectionData);

    const FIntRect ViewRect = ProjectionData.GetConstrainedViewRect();
    TArray<FVector2D> Corners;
    Corners.Add(FVector2D(ViewRect.Min.X, ViewRect.Min.Y));               // TL
    Corners.Add(FVector2D(ViewRect.Max.X - 1, ViewRect.Min.Y));               // TR
    Corners.Add(FVector2D(ViewRect.Max.X - 1, ViewRect.Max.Y - 1));           // BR
    Corners.Add(FVector2D(ViewRect.Min.X, ViewRect.Max.Y - 1));           // BL

    for (const FVector2D& ScreenPos : Corners)
    {
        FVector WorldOrigin, WorldDir;
        if (PC->DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldOrigin, WorldDir))
        {
            if (!FMath::IsNearlyZero(WorldDir.Z))
            {
                // Ebene: Z = 0  → Ebenennormal = (0,0,1)
                const FVector PlaneNormal = FVector::UpVector;
                const float   PlaneD = 0.0f;   // Ebene bei Z=0

                // Geradenparameter: Origin + t * Dir
                const float denom = FVector::DotProduct(WorldDir, PlaneNormal);

                if (FMath::Abs(denom) > KINDA_SMALL_NUMBER)
                {
                    const float t = -(FVector::DotProduct(WorldOrigin, PlaneNormal) - PlaneD) / denom;
                    FVector Hit = WorldOrigin + t * WorldDir;

                    Out.Add(WorldToMinimap(Hit));
                }
            }
        }
    }

    return Out;
}

void UMinimap::DrawMinimapToTexture(UCanvas* Canvas, int32 Width, int32 Height)
{
    //Draw minimap frame
    Canvas->K2_DrawBox(
        FVector2D(0, 0), 
        FVector2D(Width, Width), 
        Width,
        FLinearColor(0.0039f, 0.0118f, 0.0235f, 0.8f));

    FCanvasTileItem TileItem(
        FVector2D(100.0f, 0.0f),
        MapTex->GetResource(),
        FVector2D(200.0f,400.0f),
        FLinearColor(1.f, 1.f, 1.f, 1.0f));

        // Rotation einstellen
        TileItem.Rotation = FRotator(0.f, 0.f, 0.f);
        TileItem.PivotPoint = FVector2D(0.5f, 0.5f);
        TileItem.BlendMode = SE_BLEND_Translucent;
        Canvas->DrawItem(TileItem);

}

void UMinimap::MoveCameraTo(const FVector2D& MapPos)
{
    if (!CameraPawn) return;
    FVector World = MinimapToWorld(MapPos);
    CameraPawn->SetActorLocation(World);
}

void UMinimap::StartContinuousMove(const FVector2D& MapPos)
{
    DragTarget = MapPos;
    bDragging = true;
}

void UMinimap::StopContinuousMove()
{
    bDragging = false;
}
