// ===== File: UnitBuildDataSubsystem.cpp =====
#include "UnitBuildDataSubsystem.h"

void UUnitBuildDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Standard‑Bauzeiten
    BuildTimeMap.Add(ECarnageUnitType::Worker, 1.0f);
    BuildTimeMap.Add(ECarnageUnitType::Soldier, 1.0f);
}

float UUnitBuildDataSubsystem::GetBuildTimeForUnit(ECarnageUnitType UnitType) const
{
    if (const float* Time = BuildTimeMap.Find(UnitType))
    {
        return *Time;
    }
    return -1.0f;
}