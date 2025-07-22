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
