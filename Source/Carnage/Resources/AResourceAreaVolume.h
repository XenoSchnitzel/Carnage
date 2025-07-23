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

    UPROPERTY(EditAnywhere, Category = "Resource")
    int32 TotalResources = 1000;


    //This distributes resources, Less, when beeing away from the center, more when beeing close to the center
    // The extrem value 0.0f means, that resources at the border have only 10% of its original share of 100 and the ones in the center get double the share 200
    UPROPERTY(EditAnywhere, Category = "Resource", meta=(ClampMin=0.0, ClampMax=1.0))
    float DistributionUniformity = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Resource")
    TSubclassOf<AResourceNode> ResourceNodeClass;

    UPROPERTY(EditAnywhere, Category = "Resource")
    TArray<UStaticMesh*> ResourceMeshes;

    UPROPERTY(EditAnywhere, Category = "Editor")
    int32 GenerationSeed = 123;

    UFUNCTION(CallInEditor)
    void GenerateResources();

    UFUNCTION(CallInEditor)
    void ClearResources();

private:
    TArray<FVector> GeneratePointsInPolygon(int32 Count);
    AResourceNode* SpawnResourceAt(const FVector& Location, FRandomStream& Stream);
};
