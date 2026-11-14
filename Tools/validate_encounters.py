from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BREACH = (ROOT / "Source/HunterGame/Items/HunterBreachEncounterActor.cpp").read_text(encoding="utf-8")
BREACH_H = (ROOT / "Source/HunterGame/Items/HunterBreachEncounterActor.h").read_text(encoding="utf-8")
BOSS = (ROOT / "Source/HunterGame/Items/HunterBossEncounterActor.cpp").read_text(encoding="utf-8")
BOSS_H = (ROOT / "Source/HunterGame/Items/HunterBossEncounterActor.h").read_text(encoding="utf-8")

# Breach wave: start once, pace spawns, track only successfully spawned enemies,
# and wait for both the spawn queue and active enemy count before rewarding.
assert "if (bEncounterStarted || !Arin || !MonsterClass || !GetWorld())" in BREACH
assert "SpawnTimerHandle" in BREACH_H
assert "FMath::Max(0.1f, SpawnIntervalSeconds)" in BREACH
assert "SpawnedMonsters.Add(Monster)" in BREACH
assert "++RemainingEnemies" in BREACH
assert "SpawnedMonsters.RemoveSingle(Monster) == 0" in BREACH
assert "RemainingEnemies == 0 && NextSpawnIndex >= PendingSpawnTransforms.Num()" in BREACH
assert "if (bEncounterComplete)" in BREACH
assert "RewardQuestEventTag" in BREACH
assert "BP_OnEncounterCompleted()" in BREACH

# Boss encounter: a failed spawn must not mark the encounter started; a successful
# boss is tracked by identity and its defeat event is emitted once.
assert "if (!SpawnedBoss)" in BOSS
assert "BP_OnBossSpawnFailed()" in BOSS
assert "bEncounterStarted = true" in BOSS
assert "SpawnedBoss->OnMonsterDied.AddDynamic" in BOSS
assert "DeadBoss != SpawnedBoss" in BOSS
assert "if (!bEncounterStarted || bEncounterComplete || DeadBoss != SpawnedBoss)" in BOSS
assert "BossDefeatQuestEventTag" in BOSS
assert "BP_OnBossEncounterDefeated(SpawnedBoss)" in BOSS
assert "HasEncounterStarted() const" in BOSS_H
assert "IsEncounterComplete() const" in BOSS_H

print("Breach and boss encounter source checks passed.")

# Editor-authored encounter settings must be normalized before use.
assert "FMath::IsFinite(TriggerRadius)" in BREACH
assert "FMath::Clamp(TriggerRadius, 50.0f, 5000.0f)" in BREACH
assert "FMath::IsFinite(SpawnIntervalSeconds)" in BREACH
assert "FMath::Clamp(SpawnIntervalSeconds, 0.1f, 30.0f)" in BREACH
assert "FMath::Clamp(FallbackMonsterCount, 1, 20)" in BREACH
assert "FMath::Clamp(MonsterSpawnPoints.Num(), 1, 20)" in BREACH
assert "AuthoredTransform.IsValid()" in BREACH
