#!/usr/bin/env python3
"""Validate Chapter 13's persistent Chapter 12 pact consequences."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
CAMPAIGN = (ROOT / "Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp").read_text(encoding="utf-8")
TRIGGER_H = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.h").read_text(encoding="utf-8")
TRIGGER_CPP = (ROOT / "Source/HunterGame/Quests/HunterQuestTriggerActor.cpp").read_text(encoding="utf-8")
GENERATOR = (ROOT / "Tools/unreal/create_chapter13_level.py").read_text(encoding="utf-8")

for token in ('Chapter12StoryChoice == FName(TEXT("BindShadows"))',
              'Chapter12StoryChoice == FName(TEXT("FreeShadows"))',
              "CommandShadowVanguard", "ProtectFreedShadowRefugees",
              "ShadowVanguardCommanded", "ShadowRefugeesProtected"):
    assert token in CAMPAIGN, f"Chapter 13 branch objective missing: {token}"
assert "RequiredStoryChoice" in TRIGGER_H
assert "Campaign->GetChapter12StoryChoice() != RequiredStoryChoice" in TRIGGER_CPP
assert "SetActorEnableCollision(false)" in TRIGGER_CPP
for token in ('ShadowVanguardCommanded', 'ShadowRefugeesProtected',
              'BindShadows', 'FreeShadows', 'required_story_choice',
              'CH13_Shadow_Vanguard_Command', 'CH13_Shadow_Refugee_Escort'):
    assert token in GENERATOR, f"Chapter 13 branch trigger missing: {token}"
compile(GENERATOR, str(ROOT / "Tools/unreal/create_chapter13_level.py"), "exec")
print("Chapter 13 pact-dependent objectives and mutually exclusive world interactions passed.")
