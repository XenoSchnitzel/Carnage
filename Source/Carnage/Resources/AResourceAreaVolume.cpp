#include "AResourceAreaVolume.h"
#include "AResourceNode.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ThirdParty/earcut.hpp"
#include <numeric>
#include <array>

template <typename T> using Tri = std::array<T, 3>;

AResourceAreaVolume::AResourceAreaVolume()
{
    PrimaryActorTick.bCanEverTick = false;
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
    RootComponent = SplineComponent;
    SplineComponent->SetClosedLoop(true);
}

void AResourceAreaVolume::OnConstruction(const FTransform& Transform)
{
    // Optional: Automatically regenerate in editor
}

void AResourceAreaVolume::ClearResources()
{
    TArray<AActor*> Attached;
    GetAttachedActors(Attached);
    for (AActor* A : Attached)
    {
        if (A && A->IsA<AResourceNode>())
        {
            A->Destroy();
        }
    }
}

void AResourceAreaVolume::GenerateResources()
{
    ClearResources();
    if (!ResourceNodeClass) return;

    FRandomStream Stream(GenerationSeed);
    const int32 Count = FMath::Max(1, TotalResources / 100);

    TArray<FVector> Points = GeneratePointsInPolygon(Count);
    for (const FVector& P : Points)
    {
        SpawnResourceAt(P, Stream);
    }
}

void AResourceAreaVolume::SpawnResourceAt(const FVector& Location, FRandomStream& Stream)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AResourceNode* Node = GetWorld()->SpawnActor<AResourceNode>(
        ResourceNodeClass,
        Location,
        FRotator::ZeroRotator,
        Params
    );

    if (Node && ResourceMeshes.Num() > 0)
    {
        const int32 Index = Stream.RandRange(0, ResourceMeshes.Num() - 1);
        Node->SetMesh(ResourceMeshes[Index]);
        Node->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    }
}

FVector RandomPointInTriangle(const FVector2D& A, const FVector2D& B, const FVector2D& C, FRandomStream& Stream)
{
    float u = Stream.FRand();
    float v = Stream.FRand();
    if (u + v > 1.f)
    {
        u = 1.f - u;
        v = 1.f - v;
    }
    FVector2D P = A + u * (B - A) + v * (C - A);
    return FVector(P.X, P.Y, 0.f);
}

TArray<FVector> AResourceAreaVolume::GeneratePointsInPolygon(int32 Count)
{
    using Point = std::array<double, 2>;
    using Polygon = std::vector<std::vector<Point>>;

    FRandomStream Stream(GenerationSeed);

    // Convert Spline points to std::array format for earcut
    std::vector<Point> OuterPolygon;
    for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
    {
        FVector P = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        OuterPolygon.push_back({ static_cast<double>(P.X), static_cast<double>(P.Y) });
    }

    Polygon Poly = { OuterPolygon };
    std::vector<int> Indices = mapbox::earcut<int>(Poly);

    // Reconstruct triangles from indices
    TArray<std::array<Point, 3>> Triangles;
    for (size_t i = 0; i < Indices.size(); i += 3)
    {
        Triangles.Add({
            OuterPolygon[Indices[i]],
            OuterPolygon[Indices[i + 1]],
            OuterPolygon[Indices[i + 2]]
            });
    }

    // Calculate triangle areas
    std::vector<float> Areas;
    for (const auto& T : Triangles)
    {
        float Area = FMath::Abs(
            static_cast<float>(
                T[0][0] * (T[1][1] - T[2][1]) +
                T[1][0] * (T[2][1] - T[0][1]) +
                T[2][0] * (T[0][1] - T[1][1])
                ) * 0.5f
        );
        Areas.push_back(Area);
    }

    // Cumulative area distribution
    std::vector<float> Cumulative;
    std::partial_sum(Areas.begin(), Areas.end(), std::back_inserter(Cumulative));
    float TotalArea = Cumulative.back();

    // Sample random points in triangle
    TArray<FVector> Points;
    Points.Reserve(Count);

    for (int i = 0; i < Count; ++i)
    {
        float R = Stream.FRandRange(0.f, TotalArea);
        int TriIndex = static_cast<int>(std::lower_bound(Cumulative.begin(), Cumulative.end(), R) - Cumulative.begin());
        TriIndex = FMath::Clamp(TriIndex, 0, Triangles.Num() - 1);

        const auto& T = Triangles[TriIndex];

        // Sample barycentric coordinates
        float u = Stream.FRand();
        float v = Stream.FRand();
        if (u + v > 1.0f)
        {
            u = 1.0f - u;
            v = 1.0f - v;
        }

        float x = static_cast<float>(T[0][0] + u * (T[1][0] - T[0][0]) + v * (T[2][0] - T[0][0]));
        float y = static_cast<float>(T[0][1] + u * (T[1][1] - T[0][1]) + v * (T[2][1] - T[0][1]));
        float z = GetActorLocation().Z;

        // Project to terrain
        FHitResult Hit;
        FVector Start(x, y, z + 10000.f);
        FVector End(x, y, z - 10000.f);
        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
        {
            z = Hit.ImpactPoint.Z;
        }

        Points.Add(FVector(x, y, z));
    }

    return Points;
}
