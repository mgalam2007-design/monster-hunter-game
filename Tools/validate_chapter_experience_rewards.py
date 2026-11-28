from pathlib import Path

source = Path("Source/HunterGame/Campaign/HunterCampaignSubsystem.cpp").read_text(encoding="utf-8")
start = source.index("bool UHunterCampaignSubsystem::CompleteCurrentChapter()")
end = source.index("bool UHunterCampaignSubsystem::SetChapter12StoryChoice", start)
body = source[start:end]
completed_guard = body.index("Chapters[Index].bCompleted")
reward = body.index("Progression->AddExperience(100);")
completion = body.index("Chapters[Index].bCompleted = true;")
if not (completed_guard < reward < completion):
    raise SystemExit("chapter XP must be awarded only after the duplicate-completion guard and before completion is committed")
if "UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)" not in body:
    raise SystemExit("chapter completion must safely find the local player character")
if "Arin->GetProgressionComponent()" not in body:
    raise SystemExit("chapter completion must award XP through Arin's progression component")
print("Chapter completion experience reward checks passed")
