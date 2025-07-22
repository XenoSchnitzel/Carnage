#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AResourceNode.generated.h"

UCLASS()
class CARNAGE_API AResourceNode : public AActor
{
    GENERATED_BODY()

public:
    AResourceNode();

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ResourceAmount = 100.f;

    // Set the mesh from spawner at construction time
    void SetMesh(UStaticMesh* NewMesh);
};
