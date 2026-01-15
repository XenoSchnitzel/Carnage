#include "Minimap.h"
#include "../PlayerController/CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "Landscape.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"

//Local helpers
#define INITAL_ROTATION 90.0f

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

static bool IsPointInsideConvexQuad(
    int32 MouseX,
    int32 MouseY,
    const TArray<FVector2D>& Quad)
{
    check(Quad.Num() == 4);

    const FVector2D P(MouseX, MouseY);

    bool bHasPositive = false;
    bool bHasNegative = false;

    auto Cross2D = [](const FVector2D& A, const FVector2D& B)
        {
            return A.X * B.Y - A.Y * B.X;
        };

    for (int32 i = 0; i < 4; ++i)
    {
        const FVector2D& A = Quad[i];
        const FVector2D& B = Quad[(i + 1) % 4];

        const float Cross = Cross2D(B - A, P - A);

        if (Cross > 0.f) bHasPositive = true;
        else if (Cross < 0.f) bHasNegative = true;

        // Sobald beide Seiten vorkommen → draußen
        if (bHasPositive && bHasNegative)
        {
            return false;
        }
    }

    return true;
}

//class methods

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
    // TODO: Make this screen size independent
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
    MiniMapPivotCenter = (MiniMapFrameBottomRight - this->MiniMapFrameTopLeft) / 2.0f + this->MiniMapFrameTopLeft;

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

    CurrentFrameData.CameraFrustumPoints = ComputeCameraFrustum(CurrentFrameData.MinimapRotation);
    CurrentFrameData.UnitPositions.Empty();

    TArray<FVector2D> tmpMiniMapFramePoints;

    FVector2D Size = this->MiniMapFrameBox.GetSize();

    FVector2D absTopLeft = this->MiniMapFrameTopLeft + this->MiniMapTopLeft;
    FVector2D absBottomRight = this->MiniMapFrameTopLeft + this->MiniMapBottomRight;

    //Fill in with inital values
    tmpMiniMapFramePoints.Add(absTopLeft);              //TL
    tmpMiniMapFramePoints.Add(FVector2D(
        absBottomRight.X,
        absTopLeft.Y));                                 //TR
    tmpMiniMapFramePoints.Add(absBottomRight);          //BR
    tmpMiniMapFramePoints.Add(FVector2D(
        absTopLeft.X,
        absBottomRight.Y));                             //BL

    FVector2D Pivot = this->MiniMapPivotCenter;

    //Now rotate the individual points
    for (FVector2d& vec : tmpMiniMapFramePoints) {
        vec = RotateAround(vec, Pivot, - CameraPawn->GetActorRotation().Yaw - INITAL_ROTATION);
    }

    CurrentFrameData.MiniMapBorderPoints = tmpMiniMapFramePoints;

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

FVector2D UMinimap::ScreenToMinimap(int32 screen_x, int32 screen_y)
{
    //First: Rotate back around PivotPoint
    FVector2d rVec = RotateAround(FVector2D(screen_x,screen_y), this->MiniMapPivotCenter, CameraPawn->GetActorRotation().Yaw + INITAL_ROTATION);
    

    UE_LOG(LogTemp, Log,
        TEXT("ScreenToMinimap | AfterRotate rVec=(%.2f, %.2f)"),
        rVec.X, rVec.Y
    );

    //Second: remove Map-Offset, so that we have the actual minimap screen coordinates
    rVec = FVector2D(
        rVec.X - this->MiniMapFrameTopLeft.X - this->MiniMapTopLeft.X,
        rVec.Y - this->MiniMapFrameTopLeft.Y - this->MiniMapTopLeft.Y);

    UE_LOG(LogTemp, Log,
        TEXT("ScreenToMinimap | AfterOffset rVec=(%.2f, %.2f)"),
        rVec.X, rVec.Y
    );

    return rVec;
}

FVector UMinimap::MinimapToWorld(const FVector2D& MapPos) const
{
    FVector2D MinimapSize = MiniMapBottomRight - MiniMapTopLeft;

    UE_LOG(LogTemp, Log,
        TEXT("MinimapToWorld | MinimapSize=(%.2f, %.2f)"),
        MinimapSize.X, MinimapSize.Y
    );

    FVector2D Norm(MapPos.X / MinimapSize.X, MapPos.Y / MinimapSize.Y);

    UE_LOG(LogTemp, Log,
        TEXT("MinimapToWorld | Norm=(%.4f, %.4f)"),
        Norm.X, Norm.Y
    );


    FVector rVec = FVector(
        FMath::Lerp(WorldBounds.Min.X, WorldBounds.Max.X, Norm.X),
        FMath::Lerp(WorldBounds.Min.Y, WorldBounds.Max.Y, Norm.Y),
        0.f);

    UE_LOG(LogTemp, Log,
        TEXT("MinimapToWorld | WorldPos=(%.2f, %.2f, %.2f)"),
        rVec.X, rVec.Y, rVec.Z
    );

    return rVec;
}

TArray<FVector2D> UMinimap::ComputeCameraFrustum(float rotation) const
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

                    //Finally apply rotation, Camera always points up and x,y are shifted in screen space, so rotate 90 degrees counterclockwise
                    const float RotationDeg = -rotation - INITAL_ROTATION;

                    //Calculate Pivot
                    const FVector2D Pivot =
                        this->MiniMapFrameTopLeft + this->MiniMapFrameBox.GetSize() * 0.5f;

                    //World to Minimap just applies real screen coordinates again, finally apply rotation around our pivot
                    Out.Add(RotateAround(WorldToMinimap(Hit), Pivot, RotationDeg));
                }
            }
        }
    }

    return Out;
}

FUIHitInfo UMinimap::CheckIfMinimapIsHit(int screen_x, int screen_y) {
    
    FUIHitInfo hitInfo;

    TArray<FVector2D> tmpMiniMapFramePoints;

    FVector2D Size = this->MiniMapFrameBox.GetSize();

    FVector2D absTopLeft = this->MiniMapFrameTopLeft + this->MiniMapTopLeft;
    FVector2D absBottomRight = this->MiniMapFrameTopLeft + this->MiniMapBottomRight;

    //Fill in with inital values
    tmpMiniMapFramePoints.Add(absTopLeft);              //TL
    tmpMiniMapFramePoints.Add(FVector2D(
        absBottomRight.X,
        absTopLeft.Y));                                 //TR
    tmpMiniMapFramePoints.Add(absBottomRight);          //BR
    tmpMiniMapFramePoints.Add(FVector2D(
        absTopLeft.X,
        absBottomRight.Y));                             //BL

    FVector2D Pivot = FVector2D(
        this->MiniMapFrameTopLeft.X + Size.X / 2.0f,
        this->MiniMapFrameTopLeft.Y + Size.Y / 2.0f);

    //Now rotate the individual points
    for (FVector2d& vec : tmpMiniMapFramePoints) {
        vec = RotateAround(vec, Pivot,-CameraPawn->GetActorRotation().Yaw - INITAL_ROTATION);
    }

    if (IsPointInsideConvexQuad(screen_x, screen_y, tmpMiniMapFramePoints)) {
        hitInfo.HitType = EUIHitType::Minimap;
        
		FVector2D minimapPos = ScreenToMinimap(screen_x, screen_y);

		hitInfo.WorldTarget = MinimapToWorld(minimapPos);
    }
    else {
        hitInfo.HitType = EUIHitType::None;
    }

    return hitInfo;
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
