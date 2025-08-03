#include "UAlliance.h"
#include "enum/EAlliance.h"



EAlliance UAlliance::GetAllianceId() const
{
	return this->eAllianceId;
}

void UAlliance::SetAllianceId(EAlliance eNewAllianceId)
{
	this->eAllianceId = eNewAllianceId;
}

void UAlliance::AddFaction(UFactionState* newFactionState)
{
	this->FArrayFactions.Add(newFactionState);
    newFactionState->SetAllianceId(this->eAllianceId);
}

bool UAlliance::RemoveTeamFromAlliance(EFaction removalFactionState) {
    for (UFactionState* currentFactionState : this->FArrayFactions) {
        if (currentFactionState->GetFactionId() == removalFactionState)
        {
            FArrayFactions.Remove(currentFactionState);
            return true;
        }
    }

    return false;
}

bool UAlliance::IsFactionInAlliance(EFaction faction) {
    for (UFactionState* factionState : FArrayFactions) {
        if (factionState->GetFactionId() == faction) {
            return true;
        }
    }

    return false;
}

TArray<UFactionState*> UAlliance::GetAllFactions() const {
    return FArrayFactions;
}