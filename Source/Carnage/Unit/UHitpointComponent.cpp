#include "UHitpointComponent.h"

UHitpointComponent::UHitpointComponent()
{
    // Im BP war Tick aktiviert – falls du keinen Tick willst, auf false setzen.
    PrimaryComponentTick.bCanEverTick = true;
}
