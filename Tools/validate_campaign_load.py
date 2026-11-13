#!/usr/bin/env python3
"""Check that campaign loading rejects malformed save data before mutating state."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp"


def fail(message: str) -> None:
    print(f"ERROR: {message}", file=sys.stderr)
    raise SystemExit(1)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    start = source.find("bool UHunterCampaignSubsystem::LoadCampaign(")
    if start < 0:
        fail("LoadCampaign function was not found")
    body = source[start:]

    required = {
        "chapter lower/upper bounds": "SaveObject->CurrentChapterNumber > Chapters.Num()",
        "nonnegative experience": "SaveObject->ArinExperience < 0",
        "nonnegative currency": "SaveObject->ArinCurrency < 0",
        "evolution-stage bounds": "SaveObject->ArinEvolutionStage > 4",
        "completed chapter array bound": "SaveObject->CompletedChapterNumbers.Num() > Chapters.Num()",
        "duplicate completion detection": "UniqueCompletedChapters.Contains(ChapterNumber)",
        "invalid completion rejection": "ChapterNumber > Chapters.Num()",
    }
    for description, fragment in required.items():
        if fragment not in body:
            fail(f"load validation is missing {description}")

    validation_end = body.find("TSet<int32> ValidCompletedChapters;")
    mutation_start = body.find("for (FHunterChapter& Chapter : Chapters)")
    if validation_end < 0 or mutation_start < 0 or validation_end > mutation_start:
        fail("save snapshot must be fully validated before campaign state is mutated")

    for fragment in required.values():
        if body.find(fragment) > validation_end:
            fail("all snapshot validation must happen before campaign state mutation")

    print("Validated early rejection of malformed campaign snapshots before state restoration.")


if __name__ == "__main__":
    main()
