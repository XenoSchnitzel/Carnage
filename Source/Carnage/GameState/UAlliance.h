#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "enum/EAlliance.h"
#include "enum/EFaction.h"
#include "UFactionState.h"

#include "UAlliance.generated.h"

UCLASS(Blueprintable)
class CARNAGE_API UAlliance : public UObject
{
    GENERATED_BODY()

protected:

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EAlliance eAllianceId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    TArray<UFactionState*> FArrayFactions;

public:

    UFUNCTION(BlueprintCallable)
    EAlliance GetAllianceId() const;

    UFUNCTION(BlueprintCallable)
    void SetAllianceId(EAlliance allianceId);

    UFUNCTION(BlueprintCallable)
    void AddFaction(UFactionState* newFactionState);

    UFUNCTION(BlueprintCallable)
    bool RemoveTeamFromAlliance(EFaction removalFactionState);

};