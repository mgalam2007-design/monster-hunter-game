"""Generate Chapter 4's abandoned transit-gate boss encounter in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
Creates a reach objective, a native multi-phase boss encounter and a gated exit.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter04/L_CH04_TransitGate"


def native_class(name):
    cls = unreal.load_class(None, "/Script/HunterGame." + name)
    if not cls:
        raise RuntimeError("Missing native class " + name + "; compile HunterGame first.")
    return cls


def spawn(cls, location, label, rotation=None):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls, unreal.Vector(*location), rotation or unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError("Could not spawn " + label)
    actor.set_actor_label(label)
    return actor


def set_property(actor, name, value):
    try:
        actor.set_editor_property(name, value)
    except Exception as error:
        raise RuntimeError("Could not set " + name + " on " +
                           actor.get_name() + ": " + str(error))


def build_chapter_four():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter04")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH04_Abandoned_Transit_Gate")
    set_property(arena, "arena_half_width", 1800.0)
    set_property(arena, "arena_half_depth", 1800.0)
    set_property(arena, "wall_height", 720.0)

    spawn(unreal.PlayerStart, (0, -1400, 110), "CH04_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1100), "CH04_Sun",
                unreal.Rotator(-38.0, -28.0, 0.0))
    set_property(sun, "intensity", 5.5)
    sky = spawn(unreal.SkyLight, (0, 0, 800), "CH04_SkyLight")
    set_property(sky, "intensity", 0.8)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH04_Transit_Fog")
    set_property(fog, "fog_density", 0.022)

    gate_marker = spawn(trigger_class, (0, -350, 100),
                        "CH04_Reach_Sealed_Transit_Gate")
    set_property(gate_marker, "quest_event_tag", unreal.Name("TransitGateReached"))
    set_property(gate_marker, "event_amount", 1)
    set_property(gate_marker, "trigger_once", True)
    set_property(gate_marker, "destroy_after_trigger", False)
    set_property(gate_marker, "requires_interaction", False)

    boss_encounter = spawn(boss_encounter_class, (0, 450, 100),
                           "CH04_Transit_Guardian_Encounter")
    set_property(boss_encounter, "boss_class", boss_class)
    set_property(boss_encounter, "boss_defeat_quest_event_tag",
                 unreal.Name("TransitGuardianDefeated"))
    set_property(boss_encounter, "activation_radius", 520.0)
    set_property(boss_encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(650.0, 0.0, 0.0)))

    exit_actor = spawn(exit_class, (0, 1480, 100), "CH04_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 300)
    set_property(exit_actor, "completion_currency_reward", 150)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH04_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(38.0, 38.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 4 transit-gate arena with reach objective and phase-based guardian encounter.")
    unreal.log_warning("HUNTER: Build NavMesh and test boss spawning, phase changes, quest event and chapter exit in Unreal Editor.")


build_chapter_four()
