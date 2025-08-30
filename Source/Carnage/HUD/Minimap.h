#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Minimap.generated.h"

USTRUCT(BlueprintType)
struct CARNAGE_API FMinimapFrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FVector2D MinimapCenter;
    UPROPERTY(BlueprintReadOnly) float MinimapRotation = 0.f;
    UPROPERTY(BlueprintReadOnly) TArray<FVector2D> CameraFrustumPoints;
    UPROPERTY(BlueprintReadOnly) TArray<FVector2D> UnitPositions;
};

UCLASS()
class CARNAGE_API UMinimap : public UObject
{
    GENERATED_BODY()

public:
    void Initialize(class ACameraPawn* InCamera, const FVector2D& InScreenSize, const FBox2D& InWorldBounds = FBox2D(EForceInit::ForceInit));

    void Tick(float DeltaSeconds);
    FMinimapFrameData GetFrameData() const { return CurrentFrameData; }

    void MoveCameraTo(const FVector2D& MapPos);
    void StartContinuousMove(const FVector2D& MapPos);
    void StopContinuousMove();

    const FBox2D& GetWorldBounds() const { return WorldBounds; }

private:
    void UpdateFrameData();
    void DetectWorldBoundsFromLandscape();
    FVector2D WorldToMinimap(const FVector& WorldPos) const;
    FVector MinimapToWorld(const FVector2D& MapPos) const;
    TArray<FVector2D> ComputeCameraFrustum() const;

private:
    UPROPERTY() class ACameraPawn* CameraPawn;
    FVector2D ScreenSize;
    FBox2D WorldBounds;
    FMinimapFrameData CurrentFrameData;
    bool bDragging = false;
    FVector2D DragTarget;
};
