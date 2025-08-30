#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MyHUD.generated.h"

UCLASS()
class CARNAGE_API AMyHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void BeginPlay() override;
    virtual void DrawHUD() override;

private:
    UPROPERTY() class UMinimap* Minimap;

    void DrawMinimap();
    void DrawUnits();
};
