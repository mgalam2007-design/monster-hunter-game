"""Generate Chapter 16's Seven Beacons defense mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter16/L_CH16_SevenBeacons"


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


def build_chapter_16():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter16")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH16_World_Boundary_Beacon_Field")
    set_property(arena, "arena_half_width", 2250.0)
    set_property(arena, "arena_half_depth", 2200.0)
    set_property(arena, "wall_height", 900.0)
    spawn(unreal.PlayerStart, (0, -1750, 110), "CH16_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1250), "CH16_Boundary_Light",
                unreal.Rotator(-52.0, -20.0, 0.0))
    set_property(sun, "intensity", 4.5)
    sky = spawn(unreal.SkyLight, (0, 0, 850), "CH16_Boundary_Sky")
    set_property(sky, "intensity", 0.76)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH16_Atmospheric_Haze")
    set_property(fog, "fog_density", 0.026)

    beacon_points = [(-1500, -800, 100), (-1500, 250, 100), (-1000, 1250, 100),
                     (0, 1500, 100), (1000, 1250, 100), (1500, 250, 100),
                     (1500, -800, 100)]
    for index, point in enumerate(beacon_points, start=1):
        beacon = spawn(trigger_class, point, "CH16_Boundary_Beacon_%02d" % index)
        set_property(beacon, "quest_event_tag", unreal.Name("BoundaryBeaconActivated"))
        set_property(beacon, "event_amount", 1)
        set_property(beacon, "trigger_once", True)
        set_property(beacon, "destroy_after_trigger", False)
        set_property(beacon, "requires_interaction", True)
        set_property(beacon, "interaction_prompt_text", unreal.Text("E  ACTIVATE BOUNDARY BEACON"))
        set_property(beacon, "completion_prompt_text", unreal.Text("BOUNDARY BEACON ONLINE"))

    attacker_points = [(-650, -100, 110), (0, 450, 110), (650, -100, 110)]
    for index, point in enumerate(attacker_points, start=1):
        attacker = spawn(monster_class, point, "CH16_Beacon_Attacker_%02d" % index)
        set_property(attacker, "defeat_quest_event_tag", unreal.Name("BeaconAttackerDefeated"))

    exit_actor = spawn(exit_class, (0, 1780, 100), "CH16_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1200)
    set_property(exit_actor, "completion_currency_reward", 600)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter17/L_CH17_LastSanctuary"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 300), "CH16_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(45.0, 45.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 16 with seven boundary beacons and three attackers.")
    unreal.log_warning("HUNTER: Build NavMesh and test beacon interactions, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_16()
