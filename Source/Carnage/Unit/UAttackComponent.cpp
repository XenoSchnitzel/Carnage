#include "UAttackComponent.h"

UAttackComponent::UAttackComponent()
{
    // Im BP war Tick aktiviert – falls du keinen Tick willst, auf false setzen.
    PrimaryComponentTick.bCanEverTick = true;
}
