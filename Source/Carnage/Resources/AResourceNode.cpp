#include "AResourceNode.h"

AResourceNode::AResourceNode()
{
    PrimaryActorTick.bCanEverTick = false;
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = MeshComponent;
}

void AResourceNode::SetMesh(UStaticMesh* NewMesh)
{
    if (MeshComponent && NewMesh)
    {
        MeshComponent->SetStaticMesh(NewMesh);
    }
}

float AResourceNode::MineAmount(float Amount)
{
    float Returned = Amount;

    if (ResourceAmount < Amount)
    {
        Returned = ResourceAmount;

        // ?? Event feuern bevor der Node zerstört wird
        OnNodeDestroyed.Broadcast(this);

        Destroy();
    }
    else
    {
        ResourceAmount -= Amount;
    }

    return Returned;
}


