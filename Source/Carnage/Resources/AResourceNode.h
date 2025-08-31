#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AResourceAreaVolume.h"
#include "AResourceNode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourceNodeDestroyed, AResourceNode*, Node);

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

    void SetMesh(UStaticMesh* NewMesh);

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool b_isOccupied = false;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool b_isPreOccupied = false;

    UFUNCTION(BlueprintCallable, Category = "State")
    float MineAmount(float Amount);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource")
    AResourceAreaVolume* ParentArea;

    // 🔔 Event für Drohnen
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnResourceNodeDestroyed OnNodeDestroyed;
};
