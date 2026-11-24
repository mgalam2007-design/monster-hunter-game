#include "HunterPickupActor.h"

#include "ArinValeCharacter.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Quests/HunterQuestSubsystem.h"
#include "Engine/GameInstance.h"

AHunterPickupActor::AHunterPickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    PickupMesh->SetupAttachment(SceneRoot);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMesh->SetGenerateOverlapEvents(false);
    PickupMesh->SetCanEverAffectNavigation(false);

    // Keep native pickups visible even before a Blueprint assigns production art.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> ShardMesh(
        TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (ShardMesh.Succeeded())
    {
        PickupMesh->SetStaticMesh(ShardMesh.Object);
        PickupMesh->SetRelativeScale3D(FVector(0.38f, 0.38f, 0.62f));
    }

    PickupCollision = CreateDefaultSubobject<USphereComponent>(TEXT("PickupCollision"));
    PickupCollision->SetupAttachment(SceneRoot);
    PickupCollision->SetSphereRadius(115.0f);
    PickupCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    PickupCollision->SetCollisionObjectType(ECC_WorldDynamic);
    PickupCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
    PickupCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    PickupCollision->SetGenerateOverlapEvents(true);
}

void AHunterPickupActor::BeginPlay()
{
    Super::BeginPlay();
    PickupCollision->OnComponentBeginOverlap.AddDynamic(this, &AHunterPickupActor::HandleOverlap);
}

void AHunterPickupActor::SetPickupItem(const FHunterInventoryItem& NewItem)
{
    PickupItem = NewItem;
    PickupItem.Quantity = FMath::Max(1, PickupItem.Quantity);
    PickupItem.MaxStack = FMath::Max(1, PickupItem.MaxStack);
}

void AHunterPickupActor::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
    UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult& SweepResult)
{
    AArinValeCharacter* Arin = Cast<AArinValeCharacter>(OtherActor);
    if (!Arin || bPickupClaimed || PickupItem.ItemId.IsNone())
    {
        return;
    }

    // Claim before inventory delegates run: multiple component overlaps in one
    // frame must not grant the same pickup more than once.
    bPickupClaimed = true;
    UHunterInventoryComponent* Inventory = Arin->GetInventoryComponent();
    if (!Inventory || !Inventory->AddItem(PickupItem))
    {
        bPickupClaimed = false;
        return;
    }

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UHunterQuestSubsystem* Quest = GameInstance->GetSubsystem<UHunterQuestSubsystem>())
        {
            if (!QuestEventTag.IsNone())
            {
                Quest->RegisterQuestEvent(QuestEventTag, PickupItem.Quantity);
            }
        }
    }

    BP_OnPickedUp(Arin);
    if (bDestroyAfterPickup)
    {
        Destroy();
    }
}
