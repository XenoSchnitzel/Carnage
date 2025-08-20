#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EAttackType.h"
#include "UAttackComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CARNAGE_API UAttackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAttackComponent();

    // === exakt deine BP-Variablen ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "Type"))
    EAttackType Type = EAttackType::Melee;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "Value", ClampMin = "0.0"))
    float Value = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "MinRange", ClampMin = "0.0"))
    float MinRange = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "Spread", ClampMin = "0.0"))
    float Spread = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "CoolDownTime", ClampMin = "0.0"))
    float CoolDownTime = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack", meta = (DisplayName = "AttackTime", ClampMin = "0.0"))
    float AttackTime = 0.f;
};
