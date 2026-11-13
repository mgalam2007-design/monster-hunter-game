#!/usr/bin/env python3
"""Static safeguards for re-entrant quest delegate notifications."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
HEADER = ROOT / "Source/HunterGame/Quests/HunterQuestSubsystem.h"
SOURCE = ROOT / "Source/HunterGame/Quests/HunterQuestSubsystem.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def body(text: str, signature: str) -> str:
    start = text.find(signature)
    if start < 0:
        fail(f"Missing function: {signature}")
    opening = text.find("{", start)
    depth = 0
    for index in range(opening, len(text)):
        if text[index] == "{":
            depth += 1
        elif text[index] == "}":
            depth -= 1
            if depth == 0:
                return text[opening + 1:index]
    fail(f"Unclosed function: {signature}")


def main() -> None:
    header = HEADER.read_text(encoding="utf-8")
    source = SOURCE.read_text(encoding="utf-8")
    if "uint64 QuestStateRevision = 0;" not in header:
        fail("Quest subsystem has no state revision token")

    checks = {
        "StartQuest": "bool UHunterQuestSubsystem::StartQuest(",
        "RegisterQuestEvent": "bool UHunterQuestSubsystem::RegisterQuestEvent(",
        "ClearActiveQuest": "void UHunterQuestSubsystem::ClearActiveQuest(",
        "RestoreQuestState": "bool UHunterQuestSubsystem::RestoreQuestState(",
    }
    bodies = {name: body(source, signature) for name, signature in checks.items()}

    for name in ("StartQuest", "RegisterQuestEvent", "ClearActiveQuest", "RestoreQuestState"):
        if "++QuestStateRevision;" not in bodies[name]:
            fail(f"{name} does not advance the quest state revision")

    register = bodies["RegisterQuestEvent"]
    if "const uint64 UpdatedRevision = QuestStateRevision;" not in register:
        fail("RegisterQuestEvent does not capture its notification revision")
    if "QuestStateRevision != UpdatedRevision" not in register:
        fail("RegisterQuestEvent does not stop stale notifications after re-entry")

    restore = bodies["RestoreQuestState"]
    if "const TArray<FHunterObjective> NotificationSnapshot = Objectives;" not in restore:
        fail("RestoreQuestState does not notify from a stable objective snapshot")
    if "QuestStateRevision != RestoredRevision" not in restore:
        fail("RestoreQuestState does not stop stale notifications after re-entry")
    if restore.find("OnQuestChanged.Broadcast();") > restore.find("OnObjectiveUpdated.Broadcast("):
        fail("RestoreQuestState must publish the new snapshot before objective notifications")

    print("Validated quest revision tracking and re-entrant delegate notification safeguards.")


if __name__ == "__main__":
    main()
