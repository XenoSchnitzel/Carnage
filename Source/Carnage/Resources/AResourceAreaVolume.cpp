#include "AResourceAreaVolume.h"
#include "AResourceNode.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "ThirdParty/earcut.hpp"
#include <numeric>
#include <array>
#include "Kismet/GameplayStatics.h"

static bool IsPointInPolygon(const FVector2D& P, const TArray<FVector2D>& Poly)
{
    bool bInside = false;
    int32 Count = Poly.Num();
    for (int32 i = 0, j = Count - 1; i < Count; j = i++)
    {
        const FVector2D& Pi = Poly[i];
        const FVector2D& Pj = Poly[j];

        if (((Pi.Y > P.Y) != (Pj.Y > P.Y)) &&
            (P.X < (Pj.X - Pi.X) * (P.Y - Pi.Y) / (Pj.Y - Pi.Y) + Pi.X))
        {
            bInside = !bInside;
        }
    }
    return bInside;
}

// Uniform distribution inside an annulus (ring) between MinRadius and MaxRadius
// Density per area is uniform
static FVector2D SampleUniformInAnnulus(FRandomStream& Stream, float MinRadius, float MaxRadius)
{
    float u = Stream.FRand();
    // sqrt ensures uniform density per area
    float r = FMath::Sqrt(
        MinRadius * MinRadius + u * (MaxRadius * MaxRadius - MinRadius * MinRadius)
    );

    float angle = Stream.FRandRange(0.f, 2 * PI);

    return FVector2D(
        r * FMath::Cos(angle),
        r * FMath::Sin(angle)
    );
}

// Gaussian falloff inside annulus [MinRadius, MaxRadius]
// Mean = MinRadius, MaxRadius = Mean + 3*Sigma
// Density per area is correct
static FVector2D SampleGaussianInAnnulus(FRandomStream& Stream, float MinRadius, float MaxRadius, float distributionBias = 1.0f)
{
    if (distributionBias < 0.001f) {
        distributionBias = 0.001f;
    }

    float Sigma = distributionBias * (MaxRadius - MinRadius) / 3.0f;

    float x, y;
    {
        // Box–Muller in 2D
        float u1 = FMath::Max(Stream.FRand(), SMALL_NUMBER);
        float u2 = Stream.FRand();

        float r = FMath::Sqrt(-2.0f * FMath::Loge(u1));
        float theta = 2 * PI * u2;

        x = r * FMath::Cos(theta);
        y = r * FMath::Sin(theta);
    }

    // compute radius from Gaussian
    float gaussR = MinRadius + FMath::Abs(x) * Sigma; // radial symmetric
    gaussR = FMath::Clamp(gaussR, MinRadius, MaxRadius);

    float angle = Stream.FRandRange(0.f, 2 * PI);

    return FVector2D(
        gaussR * FMath::Cos(angle),
        gaussR * FMath::Sin(angle)
    );
}

// UD = Uniform Distribution value [0..1]
// UD = 0   -> pure Gaussian (concentrated at center)
// UD = 0.5 -> balanced Gaussian full radius
// UD = 1   -> pure Uniform (full radius)
static FVector2D CalcPointByDistributionType(FRandomStream& Stream, const FVector2D& Center, float Radius, float UD)
{
    if (UD > 0.5f)
    {
        float randomFloat = Stream.FRand();
        float normalizedUD = (UD - 0.5f) * 2.0f;  // 0..1
        float cutRadius = normalizedUD * Radius;  // inner part = uniform

        if (randomFloat < normalizedUD)
        {
            // uniform in inner circle [0, cutRadius]
            return Center + SampleUniformInAnnulus(Stream, 0.0f, cutRadius);
        }
        else
        {
            // gaussian in outer ring [cutRadius, Radius]
            return Center + SampleGaussianInAnnulus(Stream, cutRadius, Radius);
        }
    }
    else
    {
        // pure gaussian on full radius
        return Center + SampleGaussianInAnnulus(Stream, 0.0f, Radius, UD * 2.0f);
    }
}

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

AResourceNode* AResourceAreaVolume::GetNextUnoccupiedResourceNode(FVector location)
{
    AResourceNode* NearestTotallyUnoccupied = nullptr;
    AResourceNode* NearestPreOccupied = nullptr;
    float MinDistSqrNearestTotallyUnoccupied = TNumericLimits<float>::Max();
    float MinDistSqrNearestPreOccupied = TNumericLimits<float>::Max();

    for (AResourceNode* Node : Nodes)
    {
        if (!Node) {
            continue;
        }

        if (!Node->b_isOccupied)
        {
            float DistSqr = FVector::DistSquared(Node->GetActorLocation(), location);
         
            if (!Node->b_isPreOccupied) {
                if (DistSqr < MinDistSqrNearestPreOccupied)
                {
                    MinDistSqrNearestPreOccupied = DistSqr;
                    NearestPreOccupied = Node;
                }
            }
         
            if (DistSqr < MinDistSqrNearestTotallyUnoccupied)
            {
                MinDistSqrNearestTotallyUnoccupied = DistSqr;
                NearestTotallyUnoccupied = Node;
            }
        }
    }

    if (NearestPreOccupied) {
        NearestPreOccupied->b_isPreOccupied = true;
        return NearestPreOccupied;
    }

    if (NearestTotallyUnoccupied) {
        NearestTotallyUnoccupied->b_isPreOccupied = true;
        return NearestTotallyUnoccupied;
    }

    return nullptr;
}

void AResourceAreaVolume::GenerateResources()
{
    ClearResources();
    if (!ResourceNodeClass) return;

    Nodes.Empty();   // <--- clear old List

    FRandomStream Stream(GenerationSeed);

    // Anzahl an Sample-Punkten (über TotalResources steuerbar)
    const int32 Count = FMath::Max(1, TotalResources / 100);

    // Punkte im Polygon generieren (gleichmäßig verteilt)
    TArray<FVector> Points = GeneratePointsInPolygon(Count);

    // Bounding Box für Berechnung von Center und Extent
    FBox2D Bounds(EForceInit::ForceInit);
    for (const FVector& P : Points)
    {
        Bounds += FVector2D(P.X, P.Y);
    }

    const FVector2D Center = Bounds.GetCenter();
    const FVector2D Extent = Bounds.GetExtent();

    // Sigma-Werte aus UniformDistanceDistribution ableiten
    float SigmaX = FMath::Lerp(Extent.X * 0.05f, Extent.X, UniformDistanceDistribution);
    float SigmaY = FMath::Lerp(Extent.Y * 0.05f, Extent.Y, UniformDistanceDistribution);

    float Value_Max = -FLT_MAX;
    float Value_Min = FLT_MAX;

    for (const FVector& P : Points)
    {
        float dx = P.X - Center.X;
        float dy = P.Y - Center.Y;

        // Gaußfunktion für Abstand zum Zentrum
        float Weight = FMath::Exp(
            -0.5f * (FMath::Square(dx / SigmaX) + FMath::Square(dy / SigmaY))
        );

        // Mit Zufall verwerfen -> Bias zur Mitte
        if (Stream.FRand() > Weight)
        {
            continue;
        }

        // -------------------------------
        // Resource Amount nach Value-Distribution
        const float Distance = FVector2D::Distance(FVector2D(P.X, P.Y), Center);
        
        const float Normalized = FMath::Clamp(Distance / Extent.Size(), 0.f, 1.f);
        

        const float MinFactor = 0.1f;  // Rand
        const float MaxFactor = 2.0f;  // Zentrum
        const float NonUniformValue = FMath::Lerp(MaxFactor, MinFactor, Normalized);
        const float FinalValueFactor = FMath::Lerp(1.0f, NonUniformValue, 1.0f - UniformValueDistribution);

        if (Value_Max < FinalValueFactor) {
            Value_Max = FinalValueFactor;
        } 

        if (Value_Min > FinalValueFactor) {
            Value_Min = FinalValueFactor;
        }
       

        float AdjustedAmount = 100.f * FinalValueFactor;

        UE_LOG(LogTemp, Log, TEXT("Normalized: %f FinalValueFactor: %f"), Normalized, FinalValueFactor);
       
        // Node spawnen
        AResourceNode* Node = SpawnResourceAt(P, Stream);
        Node->ResourceAmount = AdjustedAmount;
        Node->ParentArea = this;
        Nodes.Add(Node);
    }

    for (AResourceNode* Node : Nodes)
    {
        if (Node)
        {
            float ResourceAmountNormalized = (Node->AdjustedAmount - Value_Min) / (Value_Max - Value_Min);
            float ScaleFactor = FMath::Lerp(ResourceScaleMin, ResourceScaleMax, ResourceAmountNormalized);
            UE_LOG(LogTemp, Log, TEXT("ResourceAmount: %f"), Node->ResourceAmount);
            UE_LOG(LogTemp, Log, TEXT("ResourceAmountNormalized: %f ScaleFactor: %f"), ResourceAmountNormalized, ScaleFactor);
            Node->SetActorScale3D(FVector(ScaleFactor));
        }
    }
}


AResourceNode* AResourceAreaVolume::SpawnResourceAt(const FVector& Location, FRandomStream& Stream)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FRotator RandomRot(0.f, Stream.FRandRange(0.f, 360.f), 0.f);

    AResourceNode* Node = GetWorld()->SpawnActor<AResourceNode>(
        ResourceNodeClass,
        Location,
        RandomRot,
        Params
    );

    if (Node && ResourceMeshes.Num() > 0)
    {
        const int32 Index = Stream.RandRange(0, ResourceMeshes.Num() - 1);
        Node->SetMesh(ResourceMeshes[Index]);
        Node->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
    }

    return Node;
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
    FRandomStream Stream(GenerationSeed);
    TArray<FVector> Points;
    Points.Reserve(Count);

    // --- Build polygon from spline ---
    TArray<FVector2D> Polygon;
    for (int32 i = 0; i < SplineComponent->GetNumberOfSplinePoints(); ++i)
    {
        FVector P = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
        Polygon.Add(FVector2D(P.X, P.Y));
    }

    // --- Bounds as approximation for center and radius ---
    FBox2D Bounds(EForceInit::ForceInit);
    for (const FVector2D& P : Polygon)
    {
        Bounds += P;
    }
    const FVector2D Center = Bounds.GetCenter();
    const float Radius = Bounds.GetExtent().Size();

    // --- Generate points ---
    int32 Generated = 0;
    int32 Safety = 0;
    while (Generated < Count && Safety < Count * 20)
    {
        Safety++;

        FVector2D Candidate = CalcPointByDistributionType(Stream, Center, Radius, UniformDistanceDistribution);

        // ensure inside polygon
        if (IsPointInPolygon(Candidate, Polygon))
        {
            // project Z onto terrain
            float z = GetActorLocation().Z;
            FHitResult Hit;
            FVector Start(Candidate.X, Candidate.Y, z + 10000.f);
            FVector End(Candidate.X, Candidate.Y, z - 10000.f);
            if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
            {
                z = Hit.ImpactPoint.Z;
            }

            Points.Add(FVector(Candidate.X, Candidate.Y, z));
            Generated++;
        }
    }

    return Points;
}
