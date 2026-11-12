#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HunterVendorActor.generated.h"

class USphereComponent;
class UHunterShopComponent;
class UHunterShopWidget;
class AArinValeCharacter;
class APlayerController;

/**
 * Proximity-based vendor interaction.
 * Press InteractKey while inside the vendor radius to open the configured shop widget.
 */
UCLASS()
class HUNTERGAME_API AHunterVendorActor : public AActor
{
    GENERATED_BODY()

public:
    AHunterVendorActor();

    UFUNCTION(BlueprintPure, Category="Vendor")
    UHunterShopComponent* GetShopComponent() const { return ShopComponent; }

    UFUNCTION(BlueprintCallable, Category="Vendor")
    void OpenVendor(AArinValeCharacter* Buyer);

    UFUNCTION(BlueprintCallable, Category="Vendor")
    void CloseVendor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vendor")
    TObjectPtr<USphereComponent> InteractionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Vendor")
    TObjectPtr<UHunterShopComponent> ShopComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Vendor|UI")
    TSubclassOf<UHunterShopWidget> ShopWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vendor|Interaction")
    FKey InteractKey = EKeys::E;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vendor|Interaction", meta=(ClampMin="50.0"))
    float InteractionRadius = 250.0f;

    UFUNCTION(BlueprintImplementableEvent, Category="Vendor")
    void BP_OnVendorNearby(bool bIsNearby);

    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void HandleInteractPressed();
    void HandleClosePressed();

private:
    TWeakObjectPtr<AArinValeCharacter> NearbyBuyer;
    TWeakObjectPtr<APlayerController> NearbyController;

    UPROPERTY(Transient)
    TObjectPtr<UHunterShopWidget> ActiveShopWidget = nullptr;

    bool bInputBound = false;
};
