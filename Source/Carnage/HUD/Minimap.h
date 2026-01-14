#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/CanvasRenderTarget2D.h"
#include "Engine/Canvas.h"
#include "EUIHitType.h"
#include "Minimap.generated.h"

#define CARNAGE_MINIMAP_X       200.0f
#define CARNAGE_MINIMAP_Y       1400.0f
#define CARNAGE_MINIMAP_SIZE    400.0f

USTRUCT(BlueprintType)
struct CARNAGE_API FUIHitInfo
{
    GENERATED_BODY()

public:

    // Was wurde getroffen?
    UPROPERTY(BlueprintReadOnly)
    EUIHitType HitType = EUIHitType::None;

    // Zielposition in der Welt (z.B. für Kamera)
    UPROPERTY(BlueprintReadOnly)
    FVector WorldTarget = FVector::ZeroVector;

    // Convenience
    bool IsValid() const
    {
        return HitType != EUIHitType::None;
    }

    static FUIHitInfo MakeMinimapHit(const FVector& InWorldTarget)
    {
        FUIHitInfo Info;
        Info.HitType = EUIHitType::Minimap;
        Info.WorldTarget = InWorldTarget;
        return Info;
    }

    static FUIHitInfo MakeNone()
    {
        return FUIHitInfo();
    }
};

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
    UPROPERTY(BlueprintReadOnly) TArray<FVector2D> MiniMapFramePoints;
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
    FUIHitInfo CheckIfMinimapIsHit(int screen_x, int screen_y);

    const FBox2D& GetWorldBounds() const { return WorldBounds; }

    // In deiner HUD-Klasse oder Minimap-Klasse
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
    UTexture* MapTex;

private:
    void UpdateFrameData();
    void DetectWorldBoundsFromLandscape();

    FVector2D ScreenToMinimap(int32 screen_x, int32 screen_y);
    FVector2D WorldToMinimap(const FVector& WorldPos) const;
    FVector MinimapToWorld(const FVector2D& MapPos) const;
    TArray<FVector2D> ComputeCameraFrustum(float rotation) const;

private:
    UPROPERTY() class ACameraPawn* CameraPawn;

    //Monitor screen size
    FVector2D ScreenSize;
    //Map boundaries in map units
    FBox2D WorldBounds;

    //Minimap absolute screen coordinates inital values (unrotated)
    FVector2D MiniMapFrameTopLeft;
    FBox2D MiniMapFrameBox;

    //Local coordinates in the minimap - inital values (unrotated) - can be smaller than the surrounding frame if map is no square
    //e.g MiniMapTopLeft = (0,100) and MiniMapBottomRight = (400,300) if the map fills 400 width but only has 200 height
    FVector2D MiniMapTopLeft;
    FVector2D MiniMapBottomRight;
    
    //Minimap rendering data WITH current rotation, passed to HUD for drawing
    FMinimapFrameData CurrentFrameData;
    bool bDragging = false;
    FVector2D DragTarget;  

    UPROPERTY()
    UCanvasRenderTarget2D *RenderTarget;

    UFUNCTION() 
    void DrawMinimapToTexture(UCanvas* Canvas, int32 Width, int32 Height);
};
