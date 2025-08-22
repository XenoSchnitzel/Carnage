#include "DecalManager.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

void UCarnageDecalManager::Initialize(FSubsystemCollectionBase& C) {
    Super::Initialize(C); 

    UE_LOG(LogTemp, Warning, TEXT("DecalManager INIT  class=%s"),
        *GetClass()->GetPathName());

    const UDecalLibrary* Lib = LoadLibrary();
    if (!Lib)
    {
        UE_LOG(LogTemp, Warning, TEXT("DecalManager: Library NOT set/loaded"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("DecalManager: Library=%s  entries=%d"),
        *Lib->GetPathName(), Lib->Decals.Num());

    for (const auto& It : Lib->Decals)
    {
        UE_LOG(LogTemp, Warning, TEXT("  - Tag=%s  surfaces=%d"),
            *It.Key.ToString(), It.Value.SurfaceMats.Num());
    }
}
void UCarnageDecalManager::Deinitialize() { Super::Deinitialize(); }

bool UCarnageDecalManager::ShouldCreateSubsystem(UObject* Outer) const {
    const UWorld* W = Outer ? Outer->GetWorld() : nullptr;
    if (!W) return false;
    const bool bPlayable = (W->WorldType == EWorldType::Game || W->WorldType == EWorldType::PIE);
    if (!bPlayable || W->GetNetMode() == NM_DedicatedServer) return false;

    // Create only if this is NOT the native class (i.e. BP child), or if a Library is set on the CDO
    const bool bIsNative = GetClass() == StaticClass();
    return !bIsNative || !Library.IsNull();
}

void UCarnageDecalManager::SetLibrary(UDecalLibrary* InLib) { Library = InLib; }

const UDecalLibrary* UCarnageDecalManager::LoadLibrary() const {
    return Library.IsNull() ? nullptr : Library.LoadSynchronous();
}

const FDecalDef* UCarnageDecalManager::FindDef(const FGameplayTag& Tag) const {
    if (const UDecalLibrary* Lib = LoadLibrary()) {
        return Lib->Decals.Find(Tag);
    }
    return nullptr;
}

UMaterialInterface* UCarnageDecalManager::ResolveMaterial(const FGameplayTag& Tag, EPhysicalSurface Surface) const {
    const UDecalLibrary* Lib = LoadLibrary();
    if (!Lib) { UE_LOG(LogTemp, Warning, TEXT("DM: no library")); return nullptr; }

    const FDecalDef* Def = Lib->Decals.Find(Tag);
    if (!Def) { UE_LOG(LogTemp, Warning, TEXT("DM: tag NOT found: %s"), *Tag.ToString()); return nullptr; }

    if (UMaterialInterface* const* M = Def->SurfaceMats.Find(Surface)) return *M;
    if (UMaterialInterface* const* M0 = Def->SurfaceMats.Find(SurfaceType_Default)) return *M0;

    UE_LOG(LogTemp, Warning, TEXT("DM: tag %s has NO material for surface %d (no Default fallback)"),
        *Tag.ToString(), (int)Surface);
    return nullptr;
}

void UCarnageDecalManager::ResolveParams(const FDecalDef* Def,
    float SizeOverride, float LifeOverride, float FadeOverride, float RandomYawOverride,
    float& OutSize, float& OutLife, float& OutFade, float& OutYawRange) const
{
    OutSize = (SizeOverride > 0.f) ? SizeOverride : (Def ? Def->DefaultSize : 24.f);
    OutLife = (LifeOverride >= 0.f) ? LifeOverride : (Def ? Def->DefaultLife : 6.f);

    const float DefaultFade = (Def && Def->DefaultFadeScreenSize > 0.f) ? Def->DefaultFadeScreenSize : GlobalFadeScreenSize;
    OutFade = (FadeOverride >= 0.f) ? FadeOverride : DefaultFade;

    const float ManagerYaw = bAllowRandomYaw ? RandomYawMaxAbs : 0.f;
    OutYawRange = (RandomYawOverride >= 0.f) ? RandomYawOverride : ManagerYaw;
}

UDecalComponent* UCarnageDecalManager::SpawnDecalByTagAtHit(const FHitResult& Hit, FGameplayTag Tag,
    float SizeOv, float LifeOv, float FadeOv, float RandYawOv)
{
    if (!GetWorld()) return nullptr;

    const EPhysicalSurface Surf = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
    UMaterialInterface* Mat = ResolveMaterial(Tag, Surf);
    if (!Mat) return nullptr;

    const FRotator BaseRot = UKismetMathLibrary::MakeRotFromZ(Hit.ImpactNormal);

    float Size, Life, Fade, Yaw;
    ResolveParams(FindDef(Tag), SizeOv, LifeOv, FadeOv, RandYawOv, Size, Life, Fade, Yaw);

    FRotator Rot = BaseRot;
    if (Yaw > 0.f) Rot.Yaw += FMath::FRandRange(-Yaw, Yaw);

    UDecalComponent* Decal = UGameplayStatics::SpawnDecalAttached(
        Mat,                     // UMaterialInterface*
        FVector(64.f, 128.f, 128.f),       // Decal size (X=thickness, Y/Z=width/height)
        Hit.GetComponent(),                      // USceneComponent* to attach to
        NAME_None,                         // Optional socket
        Hit.ImpactPoint,                   // Relative location
        Hit.ImpactNormal.Rotation(),       // Rotation so decal faces the surface
        EAttachLocation::KeepWorldPosition,
        10.0f                              // Life span in seconds
    );

    //UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation()
    //    GetWorld(), Mat, FVector(Size, Size, Size), Hit.ImpactPoint, Rot, Life);

    if (Decal && Fade > 0.f) {
        Decal->SetFadeScreenSize(0.0f);
        Decal->SetFadeOut(0, 20.0f, false);
    }
    return Decal;
}

UDecalComponent* UCarnageDecalManager::SpawnDecalByTagAtLocation(const FVector& Loc, const FVector& Normal, FGameplayTag Tag,
    EPhysicalSurface SurfForLookup, float SizeOv, float LifeOv, float FadeOv, float RandYawOv)
{
    if (!GetWorld()) return nullptr;

    UMaterialInterface* Mat = ResolveMaterial(Tag, SurfForLookup);
    if (!Mat) return nullptr;

    const FRotator BaseRot = UKismetMathLibrary::MakeRotFromZ(Normal);

    float Size, Life, Fade, Yaw;
    ResolveParams(FindDef(Tag), SizeOv, LifeOv, FadeOv, RandYawOv, Size, Life, Fade, Yaw);

    FRotator Rot = BaseRot;
    if (Yaw > 0.f) Rot.Yaw += FMath::FRandRange(-Yaw, Yaw);

    UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
        GetWorld(), Mat, FVector(Size, Size, Size), Loc, Rot, Life);

    if (Decal && Fade > 0.f) Decal->SetFadeScreenSize(Fade);
    return Decal;
}
