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

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
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

    const float HalfWidth = FMath::Max(800.0f, ArenaHalfWidth);
    const float HalfDepth = FMath::Max(800.0f, ArenaHalfDepth);
    const float Height = FMath::Max(300.0f, WallHeight);
    const float WallCenterZ = Height * 0.5f;

    // Floor and perimeter.
    AddPiece(TEXT("ArenaFloor"), FVector(0, 0, -12), FVector(HalfWidth / 50.0f, HalfDepth / 50.0f, 0.24f));
    AddPiece(TEXT("ArenaWestWall"), FVector(-HalfWidth, 0, WallCenterZ), FVector(0.5f, HalfDepth / 50.0f, Height / 100.0f));
    AddPiece(TEXT("ArenaEastWall"), FVector(HalfWidth, 0, WallCenterZ), FVector(0.5f, HalfDepth / 50.0f, Height / 100.0f));
    AddPiece(TEXT("ArenaFrontWall"), FVector(0, -HalfDepth, WallCenterZ), FVector(HalfWidth / 50.0f, 0.5f, Height / 100.0f));

    // Split rear wall leaves a central opening for the breach gate.
    AddPiece(TEXT("ArenaRearWest"), FVector(-HalfWidth * 0.66f, HalfDepth, WallCenterZ), FVector(HalfWidth * 0.34f / 50.0f, 0.5f, Height / 100.0f));
    AddPiece(TEXT("ArenaRearEast"), FVector(HalfWidth * 0.66f, HalfDepth, WallCenterZ), FVector(HalfWidth * 0.34f / 50.0f, 0.5f, Height / 100.0f));

    // Gate frame.
    AddPiece(TEXT("BreachGateLeft"), FVector(-360, HalfDepth - 24, Height * 0.38f), FVector(0.8f, 0.8f, Height * 0.38f / 50.0f));
    AddPiece(TEXT("BreachGateRight"), FVector(360, HalfDepth - 24, Height * 0.38f), FVector(0.8f, 0.8f, Height * 0.38f / 50.0f));
    AddPiece(TEXT("BreachGateLintel"), FVector(0, HalfDepth - 24, Height * 0.82f), FVector(8.0f, 0.8f, Height * 0.08f / 50.0f));

    // Four chunky cover pillars make the first combat space less empty.
    AddPiece(TEXT("ArenaPillarNW"), FVector(-HalfWidth * 0.68f, HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarNE"), FVector(HalfWidth * 0.68f, HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarSW"), FVector(-HalfWidth * 0.68f, -HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));
    AddPiece(TEXT("ArenaPillarSE"), FVector(HalfWidth * 0.68f, -HalfDepth * 0.62f, WallCenterZ * 0.65f), FVector(1.4f, 1.4f, Height * 0.65f / 100.0f));

    GateLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BreachGateLight"));
    GateLight->SetupAttachment(SceneRoot);
    GateLight->SetRelativeLocation(FVector(0, HalfDepth - 100, Height * 0.55f));
    GateLight->SetLightColor(FLinearColor(0.08f, 0.38f, 1.0f));
    GateLight->SetIntensity(6500.0f);
    GateLight->SetAttenuationRadius(1400.0f);
    GateLight->SetCastShadows(false);
}
