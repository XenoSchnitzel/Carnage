// ===== File: UnitBuildDataSubsystem.h =====
#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ECarnageUnitType.h"
#include "UnitBuildDataSubsystem.generated.h"

/**
 * Subsystem hält Bauzeit‑Mapping pro UnitType (später auch Kosten, Tech‑Prereqs etc.).
 */
UCLASS()
class CARNAGE_API UUnitBuildDataSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Liefert die Bauzeit in Sekunden; <0 wenn unbekannt */
    UFUNCTION(BlueprintCallable)
        float GetBuildTimeForUnit(ECarnageUnitType UnitType) const;

private:
    TMap<ECarnageUnitType, float> BuildTimeMap;
};