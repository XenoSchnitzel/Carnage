#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Materials/MaterialInterface.h"
#include "DecalLibrary.generated.h"

USTRUCT(BlueprintType)
struct CARNAGE_API FDecalDef {
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<TEnumAsByte<EPhysicalSurface>, UMaterialInterface*> SurfaceMats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "1.0"))
    float DefaultSize = 24.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float DefaultLife = 6.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0"))
    float DefaultFadeScreenSize = 0.01f;
};

UCLASS(BlueprintType)
class CARNAGE_API UDecalLibrary : public UDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FGameplayTag, FDecalDef> Decals;
};
