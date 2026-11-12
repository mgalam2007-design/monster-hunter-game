#include "HunterPickupActor.h"

#include "ArinValeCharacter.h"
#include "Components/HunterInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
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
    if (!Arin || PickupItem.ItemId.IsNone())
    {
        return;
    }

    UHunterInventoryComponent* Inventory = Arin->GetInventoryComponent();
    if (!Inventory || !Inventory->AddItem(PickupItem))
    {
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
