#include "Minimap.h"
#include "../PlayerController/CameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EngineUtils.h"
#include "Landscape.h"

void UMinimap::Initialize(ACameraPawn* InCamera, const FVector2D& InScreenSize, const FBox2D& InWorldBounds)
{
    CameraPawn = InCamera;
    ScreenSize = InScreenSize;

    if (InWorldBounds.bIsValid)
    {
        WorldBounds = InWorldBounds;
    }
    else
    {
        DetectWorldBoundsFromLandscape();
    }

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

    CurrentFrameData.MinimapRotation = CameraPawn->GetActorRotation().Yaw;
    CurrentFrameData.MinimapCenter   = FVector2D(100,100);

    CurrentFrameData.CameraFrustumPoints = ComputeCameraFrustum();
    CurrentFrameData.UnitPositions.Empty();
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
    FVector2D Norm(
        (WorldPos.X - WorldBounds.Min.X) / (WorldBounds.Max.X - WorldBounds.Min.X),
        (WorldPos.Y - WorldBounds.Min.Y) / (WorldBounds.Max.Y - WorldBounds.Min.Y)
    );
    return FVector2D(Norm.X * ScreenSize.X, Norm.Y * ScreenSize.Y);
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

    const FVector CamLoc = CameraPawn->TopDownCamera->GetComponentLocation();
    const FRotator CamRot = CameraPawn->TopDownCamera->GetComponentRotation();
    const float FOV = CameraPawn->TopDownCamera->FieldOfView;

    const float HalfFOV = FMath::DegreesToRadians(FOV * 0.5f);
    FVector Forward = CamRot.Vector();
    FVector Right   = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Y);
    FVector Up      = FRotationMatrix(CamRot).GetScaledAxis(EAxis::Z);

    TArray<FVector> FrustumDirs;
    FrustumDirs.Add(Forward + (Right * FMath::Tan(HalfFOV) + Up * FMath::Tan(HalfFOV)));
    FrustumDirs.Add(Forward + (-Right * FMath::Tan(HalfFOV) + Up * FMath::Tan(HalfFOV)));
    FrustumDirs.Add(Forward + (Right * FMath::Tan(HalfFOV) - Up * FMath::Tan(HalfFOV)));
    FrustumDirs.Add(Forward + (-Right * FMath::Tan(HalfFOV) - Up * FMath::Tan(HalfFOV)));

    for (FVector Dir : FrustumDirs)
    {
        Dir.Normalize();
        float T = -CamLoc.Z / Dir.Z;
        FVector Hit = CamLoc + Dir * T;
        Out.Add(WorldToMinimap(Hit));
    }
    return Out;
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
