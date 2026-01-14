#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "EUIHitType.h"
#include "Minimap.h"
#include "ACarnageHUD.generated.h"


UCLASS()
class CARNAGE_API ACarnageHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

    UFUNCTION(BlueprintCallable, Category = "UI")
    FUIHitInfo CheckIfUIisHit(int screen_x, int screen_y);

private:
    UPROPERTY() class UMinimap* Minimap;

    void InitMinimap();


    void DrawMinimap();
    void DrawUnits();
};
