from pathlib import Path

source = Path("Source/HunterGame/Characters/ArinProgressionComponent.cpp").read_text(encoding="utf-8")
start = source.index("void UArinProgressionComponent::RestoreProgression")
end = source.index("int32 UArinProgressionComponent::GetExperienceForNextStage", start)
restore = source[start:end]
if "StageProfiles.IsEmpty()" not in restore:
    raise SystemExit("progression restore must handle an empty stage profile table")
if "CurrentStage = EArinEvolutionStage::Awakening;" not in restore:
    raise SystemExit("empty stage profile fallback must restore the safe Awakening stage")
if "StageProfiles[SafeStageIndex]" not in restore:
    raise SystemExit("non-empty progression restore must retain validated stage lookup")
if restore.index("StageProfiles.IsEmpty()") > restore.index("StageProfiles[SafeStageIndex]"):
    raise SystemExit("empty stage table must be checked before indexing")
print("Arin progression restore safety checks passed")
