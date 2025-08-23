#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "DecalLibrary.h"                 // <-- wichtig: damit FDecalDef bekannt ist
#include "DecalManager.generated.h"

class UDecalComponent;
class UMaterialInterface;

UCLASS(Abstract,BlueprintType, Blueprintable)
class CARNAGE_API UCarnageDecalManager : public UWorldSubsystem {
    GENERATED_BODY()
public:
    // lifecycle
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // config
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Decals")
    TSoftObjectPtr<UDecalLibrary> Library;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decals")
    bool bAllowRandomYaw = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decals", meta = (ClampMin = "0.0", ClampMax = "180.0", EditCondition = "bAllowRandomYaw"))
    float RandomYawMaxAbs = 180.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decals", meta = (ClampMin = "0.0"))
    float GlobalFadeScreenSize = 0.01f;

    UFUNCTION(BlueprintCallable, Category = "Decals")
    void SetLibrary(UDecalLibrary* InLibrary);

    // spawn API
    UFUNCTION(BlueprintCallable, Category = "Decals")
    UDecalComponent* SpawnDecalByTagAtHit(const FHitResult& Hit, FGameplayTag DecalTag,
        float SizeOverride = -1.f, float LifeOverride = -1.f, float FadeScreenSizeOverride = -1.f, float RandomYawRangeOverride = -1.f);

    UFUNCTION(BlueprintCallable, Category = "Decals")
    UDecalComponent* SpawnDecalByTagAtLocation(const FVector& Location, const FVector& Normal, FGameplayTag DecalTag,
        EPhysicalSurface SurfaceForLookup = SurfaceType_Default,
        float SizeOverride = -1.f, float LifeOverride = -1.f, float FadeScreenSizeOverride = -1.f, float RandomYawRangeOverride = -1.f);

private:
    // internals
    const UDecalLibrary* LoadLibrary() const;
    const FDecalDef* FindDef(const FGameplayTag& Tag) const;
    UMaterialInterface* ResolveMaterial(const FGameplayTag& Tag, EPhysicalSurface Surface) const;

    void ResolveParams(const FDecalDef* Def,
        float SizeOverride, float LifeOverride, float FadeOverride, float RandomYawOverride,
        float& OutSize, float& OutLife, float& OutFade, float& OutYawRange) const;
};
