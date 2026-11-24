"""Generate Chapter 13: Siege of the North in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter13/L_CH13_SiegeOfTheNorth"


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


def build_chapter_13():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter13")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH13_Northern_Fortress")
    set_property(arena, "arena_half_width", 2100.0)
    set_property(arena, "arena_half_depth", 2050.0)
    set_property(arena, "wall_height", 850.0)
    spawn(unreal.PlayerStart, (0, -1650, 110), "CH13_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1150), "CH13_Siege_Sun",
                unreal.Rotator(-40.0, -32.0, 0.0))
    set_property(sun, "intensity", 4.6)
    sky = spawn(unreal.SkyLight, (0, 0, 820), "CH13_Fortress_Sky")
    set_property(sky, "intensity", 0.8)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH13_Siege_Smoke")
    set_property(fog, "fog_density", 0.021)

    siege_points = [(-1250, -150, 110), (-650, 500, 110), (0, 850, 110),
                    (650, 500, 110), (1250, -150, 110)]
    for index, point in enumerate(siege_points, start=1):
        enemy = spawn(monster_class, point, "CH13_Siege_Creature_%02d" % index)
        set_property(enemy, "defeat_quest_event_tag", unreal.Name("NorthernSiegeCreatureDefeated"))

    beacon_points = [(-1000, 1250, 100), (1000, 1250, 100)]
    for index, point in enumerate(beacon_points, start=1):
        beacon = spawn(trigger_class, point, "CH13_Fortress_Beacon_%02d" % index)
        set_property(beacon, "quest_event_tag", unreal.Name("FortressBeaconActivated"))
        set_property(beacon, "event_amount", 1)
        set_property(beacon, "trigger_once", True)
        set_property(beacon, "destroy_after_trigger", False)
        set_property(beacon, "requires_interaction", True)
        set_property(beacon, "interaction_prompt_text", unreal.Text("E  RESTORE FORTRESS BEACON"))
        set_property(beacon, "completion_prompt_text", unreal.Text("DEFENSE BEACON RESTORED"))

    exit_actor = spawn(exit_class, (0, 1740, 100), "CH13_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 975)
    set_property(exit_actor, "completion_currency_reward", 490)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter14/L_CH14_BrokenCitadel"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 270), "CH13_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(42.0, 42.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 13 — Siege of the North.")
    unreal.log_warning("HUNTER: Build NavMesh and test all objectives, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_13()
