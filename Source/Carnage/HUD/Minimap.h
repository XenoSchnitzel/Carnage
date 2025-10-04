#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "Minimap.generated.h"



USTRUCT(BlueprintType)
struct CARNAGE_API FMinimapFrameData
{
    GENERATED_BODY()
    UPROPERTY(BlueprintReadOnly) UCanvasRenderTarget2D* renderTarget;
    UPROPERTY(BlueprintReadOnly) FVector2D MiniMapFrameTopLeft;
    UPROPERTY(BlueprintReadOnly) FBox2D MiniMapFrameBox;
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

    UMinimap();

    void Initialize(class ACameraPawn* InCamera, const FVector2D& InScreenSize, const FBox2D& InWorldBounds = FBox2D(EForceInit::ForceInit));

    void Tick(float DeltaSeconds);
    FMinimapFrameData GetFrameData() { this->UpdateFrameData(); return CurrentFrameData; }

    void MoveCameraTo(const FVector2D& MapPos);
    void StartContinuousMove(const FVector2D& MapPos);
    void StopContinuousMove();

    const FBox2D& GetWorldBounds() const { return WorldBounds; }

    // In deiner HUD-Klasse oder Minimap-Klasse
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
    UTexture* MapTex;

private:
    void UpdateFrameData();
    void DetectWorldBoundsFromLandscape();
    FVector2D WorldToMinimap(const FVector& WorldPos) const;
    FVector MinimapToWorld(const FVector2D& MapPos) const;
    TArray<FVector2D> ComputeCameraFrustum() const;

private:
    UPROPERTY() class ACameraPawn* CameraPawn;
    
    FVector2D MiniMapFrameTopLeft;
    FBox2D MiniMapFrameBox;

    FBox2D WorldBounds;
    FVector2D MiniMapTopLeft;
    FVector2D MiniMapBottomRight;

    FVector2D ScreenSize;
    
    FMinimapFrameData CurrentFrameData;
    bool bDragging = false;
    FVector2D DragTarget;
    
    UPROPERTY()
    UCanvasRenderTarget2D *RenderTarget;

    UFUNCTION() 
    void DrawMinimapToTexture(UCanvas* Canvas, int32 Width, int32 Height);
};
