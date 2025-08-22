#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UHitpointComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CARNAGE_API UHitpointComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHitpointComponent();

    //// === exakt deine BP-Variablen ===
    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "Type"))
    //EAttackType Type = EAttackType::Melee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float Health = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MaxHealth = 0.f;
};
