#include "HunterPrototypeArenaActor.h"

#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AHunterPrototypeArenaActor::AHunterPrototypeArenaActor()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!CubeMesh.Succeeded())
    {
        return;
    }

    auto AddPiece = [this, &CubeMesh](const FName Name, const FVector& Location, const FVector& Scale)
    {
        UStaticMeshComponent* Piece = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Piece->SetupAttachment(SceneRoot);
        Piece->SetStaticMesh(CubeMesh.Object);
        Piece->SetRelativeLocation(Location);
        Piece->SetRelativeScale3D(Scale);
        Piece->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Piece->SetGenerateOverlapEvents(false);
        Piece->SetCanEverAffectNavigation(true);
        Piece->SetMobility(EComponentMobility::Static);
        ArenaPieces.Add(Piece);
    };

    const float HalfWidth = 1800.0f;
    const float HalfDepth = 1800.0f;
    const float Height = 600.0f;
    const float WallCenterZ = Height * 0.5f;

    AddPiece(TEXT("ArenaFloor"), FVector(0, 0, -12), FVector(HalfWidth / 50.0f, HalfDepth / 50.0f, 0.24f));
    AddPiece(TEXT("ArenaWestWall"), FVector(-HalfWidth, 0, WallCenterZ), FVector(0.5f, HalfDepth / 50.0f, Height / 100.0f));
    AddPiece(TEXT("ArenaEastWall"), FVector(HalfWidth, 0, WallCenterZ), FVector(0.5f, HalfDepth / 50.0f, Height / 100.0f));
    AddPiece(TEXT("ArenaFrontWall"), FVector(0, -HalfDepth, WallCenterZ), FVector(HalfWidth / 50.0f, 0.5f, Height / 100.0f));
    AddPiece(TEXT("ArenaRearWest"), FVector(-HalfWidth * 0.66f, HalfDepth, WallCenterZ), FVector(HalfWidth * 0.34f / 50.0f, 0.5f, Height / 100.0f));
    AddPiece(TEXT("ArenaRearEast"), FVector(HalfWidth * 0.66f, HalfDepth, WallCenterZ), FVector(HalfWidth * 0.34f / 50.0f, 0.5f, Height / 100.0f));
    AddPiece(TEXT("BreachGateLeft"), FVector(-360, HalfDepth - 24, Height * 0.38f), FVector(0.8f, 0.8f, Height * 0.38f / 50.0f));
    AddPiece(TEXT("BreachGateRight"), FVector(360, HalfDepth - 24, Height * 0.38f), FVector(0.8f, 0.8f, Height * 0.38f / 50.0f));
    AddPiece(TEXT("BreachGateLintel"), FVector(0, HalfDepth - 24, Height * 0.82f), FVector(8.0f, 0.8f, Height * 0.08f / 50.0f));
    AddPiece(TEXT("ArenaPillarNW"), FVector(-HalfWidth * 0.68f, HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarNE"), FVector(HalfWidth * 0.68f, HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarSW"), FVector(-HalfWidth * 0.68f, -HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarSE"), FVector(HalfWidth * 0.68f, -HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));

    GateLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BreachGateLight"));
    GateLight->SetupAttachment(SceneRoot);
    GateLight->SetRelativeLocation(FVector(0, HalfDepth - 100.0f, Height * 0.55f));
    GateLight->SetLightColor(FLinearColor(0.08f, 0.38f, 1.0f));
    GateLight->SetIntensity(6500.0f);
    GateLight->SetAttenuationRadius(1400.0f);
    GateLight->SetCastShadows(false);
}

void AHunterPrototypeArenaActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    const float HalfWidth = FMath::IsFinite(ArenaHalfWidth) ? FMath::Clamp(ArenaHalfWidth, 800.0f, 10000.0f) : 1800.0f;
    const float HalfDepth = FMath::IsFinite(ArenaHalfDepth) ? FMath::Clamp(ArenaHalfDepth, 800.0f, 10000.0f) : 1800.0f;
    const float Height = FMath::IsFinite(WallHeight) ? FMath::Clamp(WallHeight, 300.0f, 3000.0f) : 600.0f;
    const float WallCenterZ = Height * 0.5f;

    TArray<UStaticMeshComponent*> Pieces;
    GetComponents<UStaticMeshComponent>(Pieces);
    for (UStaticMeshComponent* Piece : Pieces)
    {
        if (!IsValid(Piece))
        {
            continue;
        }

        const FName Name = Piece->GetFName();
        FVector Location = Piece->GetRelativeLocation();
        FVector Scale = Piece->GetRelativeScale3D();

        if (Name == TEXT("ArenaFloor"))
        {
            Location = FVector(0, 0, -12);
            Scale = FVector(HalfWidth / 50.0f, HalfDepth / 50.0f, 0.24f);
        }
        else if (Name == TEXT("ArenaWestWall") || Name == TEXT("ArenaEastWall"))
        {
            Location = FVector(Name == TEXT("ArenaWestWall") ? -HalfWidth : HalfWidth, 0, WallCenterZ);
            Scale = FVector(0.5f, HalfDepth / 50.0f, Height / 100.0f);
        }
        else if (Name == TEXT("ArenaFrontWall"))
        {
            Location = FVector(0, -HalfDepth, WallCenterZ);
            Scale = FVector(HalfWidth / 50.0f, 0.5f, Height / 100.0f);
        }
        else if (Name == TEXT("ArenaRearWest") || Name == TEXT("ArenaRearEast"))
        {
            Location = FVector(Name == TEXT("ArenaRearWest") ? -HalfWidth * 0.66f : HalfWidth * 0.66f, HalfDepth, WallCenterZ);
            Scale = FVector(HalfWidth * 0.34f / 50.0f, 0.5f, Height / 100.0f);
        }
        else if (Name == TEXT("BreachGateLeft") || Name == TEXT("BreachGateRight"))
        {
            Location = FVector(Name == TEXT("BreachGateLeft") ? -360.0f : 360.0f, HalfDepth - 24.0f, Height * 0.38f);
            Scale = FVector(0.8f, 0.8f, Height * 0.38f / 50.0f);
        }
        else if (Name == TEXT("BreachGateLintel"))
        {
            Location = FVector(0, HalfDepth - 24.0f, Height * 0.82f);
            Scale = FVector(8.0f, 0.8f, Height * 0.08f / 50.0f);
        }
        else if (Name == TEXT("ArenaPillarNW") || Name == TEXT("ArenaPillarNE")
            || Name == TEXT("ArenaPillarSW") || Name == TEXT("ArenaPillarSE"))
        {
            const float XSign = (Name == TEXT("ArenaPillarNW") || Name == TEXT("ArenaPillarSW")) ? -1.0f : 1.0f;
            const float YSign = (Name == TEXT("ArenaPillarNW") || Name == TEXT("ArenaPillarNE")) ? 1.0f : -1.0f;
            Location = FVector(XSign * HalfWidth * 0.68f, YSign * HalfDepth * 0.62f, WallCenterZ * 0.65f);
            Scale = FVector(1.4f, 1.4f, Height * 0.65f / 100.0f);
        }

        Piece->SetRelativeLocation(Location);
        Piece->SetRelativeScale3D(Scale);
    }

    if (IsValid(GateLight))
    {
        GateLight->SetRelativeLocation(FVector(0, HalfDepth - 100.0f, Height * 0.55f));
    }
}
