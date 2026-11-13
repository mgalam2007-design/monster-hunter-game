#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HunterQuestSubsystem.generated.h"

UENUM(BlueprintType)
enum class EHunterObjectiveType : uint8
{
    Defeat,
    Interact,
    Reach,
    Collect,
    Survive
};

USTRUCT(BlueprintType)
struct FHunterObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    FName ObjectiveId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    EHunterObjectiveType Type = EHunterObjectiveType::Defeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest")
    FName EventTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest", meta=(ClampMin="1"))
    int32 RequiredCount = 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
    int32 CurrentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Quest")
    bool bCompleted = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHunterObjectiveUpdated, FName, ObjectiveId, int32, CurrentCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHunterQuestCompleted, FName, QuestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FHunterQuestChanged);

UCLASS()
class HUNTERGAME_API UHunterQuestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Quest")
    bool StartQuest(FName NewQuestId, const TArray<FHunterObjective>& NewObjectives);

    UFUNCTION(BlueprintCallable, Category="Quest")
    bool RegisterQuestEvent(FName EventTag, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category="Quest")
    void ClearActiveQuest();

    UFUNCTION(BlueprintCallable, Category="Quest|Save")
    bool RestoreQuestState(FName QuestId, const TArray<FHunterObjective>& SavedObjectives, bool bWasCompleted);

    UFUNCTION(BlueprintPure, Category="Quest")
    bool IsQuestActive() const { return ActiveQuestId != NAME_None; }

    UFUNCTION(BlueprintPure, Category="Quest")
    bool IsQuestComplete() const { return bQuestCompleted; }

    UFUNCTION(BlueprintPure, Category="Quest")
    FName GetActiveQuestId() const { return ActiveQuestId; }

    UFUNCTION(BlueprintPure, Category="Quest")
    TArray<FHunterObjective> GetObjectives() const { return Objectives; }

    UPROPERTY(BlueprintAssignable, Category="Quest")
    FHunterObjectiveUpdated OnObjectiveUpdated;

    UPROPERTY(BlueprintAssignable, Category="Quest")
    FHunterQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category="Quest")
    FHunterQuestChanged OnQuestChanged;

private:
    UPROPERTY(VisibleAnywhere, Category="Quest")
    FName ActiveQuestId = NAME_None;

    UPROPERTY(VisibleAnywhere, Category="Quest")
    TArray<FHunterObjective> Objectives;

    UPROPERTY(VisibleAnywhere, Category="Quest")
    bool bQuestCompleted = false;
};
