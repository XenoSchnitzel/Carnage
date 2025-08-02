#include "enum/EAlliance.h"
#include "UAlliance.h"


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