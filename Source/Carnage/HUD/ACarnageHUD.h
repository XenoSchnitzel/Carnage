#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ACarnageHUD.generated.h"

UCLASS()
class CARNAGE_API ACarnageHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

private:
    UPROPERTY() class UMinimap* Minimap;

    void InitMinimap();

    void DrawMinimap();
    void DrawUnits();
};
