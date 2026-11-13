#!/usr/bin/env python3
"""Static safeguards for save-slot metadata shown by the Blueprint save menu.

This checks source structure only; it does not execute Unreal save/load code.
"""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source/HunterGame/UI/HunterSaveLoadWidget.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    required = {
        "supported save version range": "SaveObject->SaveVersion >= 1 && SaveObject->SaveVersion <= 6",
        "minimum chapter bound": "SaveObject->CurrentChapterNumber >= 1",
        "maximum chapter bound": "SaveObject->CurrentChapterNumber <= 20",
        "nonnegative experience": "SaveObject->ArinExperience >= 0",
        "nonnegative currency": "SaveObject->ArinCurrency >= 0",
        "completed chapter array size bound": "SaveObject->CompletedChapterNumbers.Num() <= 20",
        "completed chapter lower bound": "CompletedChapter < 1",
        "completed chapter upper bound": "CompletedChapter > 20",
        "duplicate completion detection": "UniqueCompletedChapters.Contains(CompletedChapter)",
        "invalid snapshot rejection": "Snapshot.bSaveExists = false;",
    }
    for description, fragment in required.items():
        if fragment not in source:
            fail(f"save-slot snapshot is missing {description}")

    if source.index("UniqueCompletedChapters.Contains(CompletedChapter)") > source.index(
        "UniqueCompletedChapters.Add(CompletedChapter)"
    ):
        fail("completed chapter duplicate check must happen before insertion")

    print("Validated save version, chapter bounds, XP/currency bounds, completed chapter count, duplicate detection, and invalid-slot rejection.")


if __name__ == "__main__":
    main()
