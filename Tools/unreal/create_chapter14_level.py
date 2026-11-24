"""Generate Chapter 14: Broken Citadel in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter14/L_CH14_BrokenCitadel"


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


def build_chapter_14():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter14")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    boss_encounter_class = native_class("HunterBossEncounterActor")
    boss_class = native_class("HunterBossBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH14_Broken_Citadel_Command_Court")
    set_property(arena, "arena_half_width", 2050.0)
    set_property(arena, "arena_half_depth", 2050.0)
    set_property(arena, "wall_height", 820.0)
    spawn(unreal.PlayerStart, (0, -1600, 110), "CH14_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1120), "CH14_Citadel_Sun",
                unreal.Rotator(-43.0, -26.0, 0.0))
    set_property(sun, "intensity", 4.3)
    sky = spawn(unreal.SkyLight, (0, 0, 800), "CH14_Citadel_Sky")
    set_property(sky, "intensity", 0.74)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH14_Battle_Dust")
    set_property(fog, "fog_density", 0.024)

    pylon_points = [(-1100, 700, 100), (0, 1150, 100), (1100, 700, 100)]
    for index, point in enumerate(pylon_points, start=1):
        pylon = spawn(trigger_class, point, "CH14_Command_Pylon_%02d" % index)
        set_property(pylon, "quest_event_tag", unreal.Name("CitadelPylonDestroyed"))
        set_property(pylon, "event_amount", 1)
        set_property(pylon, "trigger_once", True)
        set_property(pylon, "destroy_after_trigger", False)
        set_property(pylon, "requires_interaction", True)
        set_property(pylon, "interaction_prompt_text", unreal.Text("E  DESTROY COMMAND PYLON"))
        set_property(pylon, "completion_prompt_text", unreal.Text("COMMAND PYLON DESTROYED"))

    encounter = spawn(boss_encounter_class, (0, 250, 100), "CH14_CITADEL_COMMANDER_ENCOUNTER")
    set_property(encounter, "boss_class", boss_class)
    set_property(encounter, "boss_defeat_quest_event_tag", unreal.Name("CitadelCommanderDefeated"))
    set_property(encounter, "activation_radius", 620.0)
    set_property(encounter, "boss_spawn_offset",
                 unreal.Transform(location=unreal.Vector(650.0, 0.0, 0.0)))

    exit_actor = spawn(exit_class, (0, 1740, 100), "CH14_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1050)
    set_property(exit_actor, "completion_currency_reward", 525)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter15/L_CH15_WorldsUnsealed"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 270), "CH14_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(42.0, 42.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 14 — Broken Citadel.")
    unreal.log_warning("HUNTER: Build NavMesh and test all objectives, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_14()
