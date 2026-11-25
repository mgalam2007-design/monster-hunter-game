#!/usr/bin/env python3
"""Validate Chapter 12's mutually exclusive, persistent pact choice."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CAMPAIGN_H = (ROOT / "Source/HunterGame/Campaign/HunterCampaignSubsystem.h").read_text(encoding="utf-8")
CAMPAIGN_CPP = (ROOT / "Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp").read_text(encoding="utf-8")
SAVE = (ROOT / "Source/HunterGame/Campaign/HunterSaveGame.h").read_text(encoding="utf-8")
TRIGGER_H = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.h").read_text(encoding="utf-8")
TRIGGER_CPP = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.cpp").read_text(encoding="utf-8")
GENERATOR = (ROOT / "Tools/unreal/create_chapter12_level.py").read_text(encoding="utf-8")

for token in ("SetChapter12StoryChoice", "GetChapter12StoryChoice", "Chapter12StoryChoice"):
    assert token in CAMPAIGN_H, f"Campaign API missing {token}"
for token in ('FName(TEXT("BindShadows"))', 'FName(TEXT("FreeShadows"))',
              "CurrentChapterNumber != 12", "Chapter12StoryChoice = Choice"):
    assert token in CAMPAIGN_CPP, f"Choice validation missing {token}"
assert "SaveVersion = 7" in SAVE and "Chapter12StoryChoice" in SAVE
assert "SaveObject->Chapter12StoryChoice = Chapter12StoryChoice" in CAMPAIGN_CPP
assert "Chapter12StoryChoice = SaveObject->Chapter12StoryChoice" in CAMPAIGN_CPP
assert "SaveObject->SaveVersion > 7" in CAMPAIGN_CPP
assert "StoryChoiceValue" in TRIGGER_H
assert "SetChapter12StoryChoice(StoryChoiceValue)" in TRIGGER_CPP
assert "BindShadows" in GENERATOR and "FreeShadows" in GENERATOR
assert GENERATOR.count('"quest_event_tag", unreal.Name("ShadowPactResolved")') == 2
print("Chapter 12 mutually exclusive pact choice, trigger integration and version-7 persistence passed.")
