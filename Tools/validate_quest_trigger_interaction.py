from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
HEADER = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.h").read_text(encoding="utf-8")
SOURCE = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.cpp").read_text(encoding="utf-8")

for token in ("bRequiresInteraction", "InteractKey", "BP_OnInteractionAvailable", "InteractionPromptText", "InteractionPrompt"):
    assert token in HEADER, f"Quest trigger header missing {token}"
for token in (
    "OnComponentEndOverlap.AddDynamic",
    "BindKey(InteractKey, IE_Pressed",
    "HandleInteractPressed",
    "TriggerVolume->IsOverlappingActor(Arin)",
    "TryRegisterQuestEvent(Arin)",
    "ClearInteractionInput",
    "EndPlay",
    "SetVisibility(true)",
    "SetVisibility(false)",
    "SetText(InteractionPromptText)",
):
    assert token in SOURCE, f"Quest trigger source missing {token}"

interaction_body = SOURCE.split("void AHunterQuestTriggerActor::HandleInteractPressed()", 1)[1]
interaction_body = interaction_body.split("bool AHunterQuestTriggerActor::TryRegisterQuestEvent", 1)[0]
assert "TryRegisterQuestEvent(Arin)" in interaction_body
assert "->RegisterQuestEvent" not in interaction_body
print("Quest trigger interaction and overlap cleanup contract passed.")
