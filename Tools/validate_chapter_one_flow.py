from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

def read(path):
    return (ROOT / path).read_text(encoding="utf-8")

campaign = read("Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp")
monster = read("Source/HunterGame/Characters/HunterMonsterBase.cpp")
breach = read("Source/HunterGame/Items/HunterBreachEncounterActor.cpp")
pickup = read("Source/HunterGame/Items/HunterPickupActor.cpp")
exit_actor = read("Source/HunterGame/Items/HunterChapterExitActor.cpp")
quest = read("Source/HunterGame/Quests/HunterQuestSubsystem.cpp")

# Chapter 1 objective definitions must match the events emitted by gameplay actors.
assert 'Quest->StartQuest(TEXT("CH01_FirstBreach"), Objectives)' in campaign
assert 'DefeatObjective.EventTag = TEXT("MonsterDefeated")' in campaign
assert 'DefeatObjective.RequiredCount = 3' in campaign
assert 'RecoverObjective.EventTag = TEXT("BreachShardCollected")' in campaign
assert 'RecoverObjective.RequiredCount = 1' in campaign
assert "QuestSubsystem->RegisterQuestEvent(DefeatQuestEventTag, 1)" in monster
assert "Pickup->SetQuestEventTag(RewardQuestEventTag)" in breach
assert "Quest->RegisterQuestEvent(QuestEventTag, PickupItem.Quantity)" in pickup

# A pickup event is only emitted after the inventory accepts the item.
grant = pickup.index("if (!Inventory || !Inventory->AddItem(PickupItem))")
event = pickup.index("Quest->RegisterQuestEvent(QuestEventTag, PickupItem.Quantity)")
assert grant < event
assert "bPickupClaimed = false;" in pickup[grant:event]

# Quest events only advance matching, active, incomplete objectives.
assert "if (!IsQuestActive() || bQuestCompleted || EventTag.IsNone() || Amount <= 0)" in quest
assert "Objective.EventTag != EventTag" in quest

# Chapter 1 cannot complete until its exact quest is active and complete.
chapter_one_gate = campaign.index("// Chapter 1 cannot be skipped")
gate_end = campaign.index("\n    }", chapter_one_gate)
gate = campaign[chapter_one_gate:gate_end]
assert 'FName(TEXT("CH01_FirstBreach"))' in gate
assert "Quest->IsQuestComplete()" in gate

# The exit commits the chapter before saving, and does not award twice on save retry.
assert "Campaign->CompleteCurrentChapter()" in exit_actor
assert "if (!bCompletionCommitted)" in exit_actor
assert "Campaign->SaveCampaign(Arin, SaveSlotName, UserIndex)" in exit_actor

print("Chapter 1 objective/event/reward/completion contracts: PASS")
