#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AResourceAreaVolume.generated.h"

class USplineComponent;
class AResourceNode;

UCLASS()
class CARNAGE_API AResourceAreaVolume : public AActor
{
    GENERATED_BODY()

public:
    AResourceAreaVolume();

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

public:
    UPROPERTY(VisibleAnywhere)
    USplineComponent* SplineComponent;

    // AResourceAreaVolume.h
    UPROPERTY(EditAnywhere, Category = "Resource|Distribution", meta = (ClampMin = 0.0))
    float BorderFactor = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Resource|Distribution", meta = (ClampMin = 0.0))
    float CenterFactor = 2.0f;

    // --- Scaling ---
    UPROPERTY(EditAnywhere, Category = "Resource|Scaling", meta = (ClampMin = 0.01, ClampMax = 100.0))
    float ResourceScaleMin = 0.25f;

    UPROPERTY(EditAnywhere, Category = "Resource|Scaling", meta = (ClampMin = 0.01, ClampMax = 100.0))
    float ResourceScaleMax = 4.0f;

    UPROPERTY(EditAnywhere, Category = "Resource|General", meta = (ClampMin = 1))
    int32 TotalResources = 1000;

    //This distributes resources, Less, when beeing away from the center, more when beeing close to the center
    // The extrem value 0.0f means, that resources at the border have only 10% of its original share of 100 and the ones in the center get double the share 200
    UPROPERTY(EditAnywhere, Category = "Resource|General", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float UniformValueDistribution = 1.0f;

    // Controls whether resources spawn more dense in center or at border
    UPROPERTY(EditAnywhere, Category = "__Resource|Distribution", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float UniformDistanceDistribution = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Resource|General")
    TSubclassOf<AResourceNode> ResourceNodeClass;

    UPROPERTY(EditAnywhere, Category = "Resource|General")
    TArray<UStaticMesh*> ResourceMeshes;

    UPROPERTY(EditAnywhere, Category = "Resource|General")
    int32 GenerationSeed = 123;

    // --- Editor Tools ---
    UFUNCTION(CallInEditor, Category = "Resource|Tools")
    void GenerateResources();

    UFUNCTION(CallInEditor, Category = "Resource|Tools")
    void ClearResources();

private:
    TArray<FVector> GeneratePointsInPolygon(int32 Count);
    AResourceNode* SpawnResourceAt(const FVector& Location, FRandomStream& Stream);
};
