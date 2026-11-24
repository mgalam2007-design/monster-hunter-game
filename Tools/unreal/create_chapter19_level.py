"""Generate Chapter 19's Beyond the Gate mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter19/L_CH19_BeyondTheGate"


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


def build_chapter_19():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter19")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH19_Origin_Gate_Threshold")
    set_property(arena, "arena_half_width", 2250.0)
    set_property(arena, "arena_half_depth", 2250.0)
    set_property(arena, "wall_height", 980.0)
    spawn(unreal.PlayerStart, (0, -1800, 110), "CH19_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1300), "CH19_Gate_Light",
                unreal.Rotator(-52.0, -18.0, 0.0))
    set_property(sun, "intensity", 4.8)
    sky = spawn(unreal.SkyLight, (0, 0, 900), "CH19_Gate_Sky")
    set_property(sky, "intensity", 0.68)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH19_Void_Mist")
    set_property(fog, "fog_density", 0.032)

    seal_points = [(-1250, 850, 100), (0, 1400, 100), (1250, 850, 100)]
    for index, point in enumerate(seal_points, start=1):
        seal = spawn(trigger_class, point, "CH19_Origin_Gate_Seal_%02d" % index)
        set_property(seal, "quest_event_tag", unreal.Name("OriginGateSealStabilized"))
        set_property(seal, "event_amount", 1)
        set_property(seal, "trigger_once", True)
        set_property(seal, "destroy_after_trigger", False)
        set_property(seal, "requires_interaction", True)
        set_property(seal, "interaction_prompt_text", unreal.Text("E  STABILIZE ORIGIN SEAL"))
        set_property(seal, "completion_prompt_text", unreal.Text("ORIGIN SEAL STABILIZED"))

    sentinel_points = [(-650, 100, 110), (650, 100, 110)]
    for index, point in enumerate(sentinel_points, start=1):
        sentinel = spawn(monster_class, point, "CH19_ORIGIN_SENTINEL_%02d" % index)
        set_property(sentinel, "defeat_quest_event_tag", unreal.Name("OriginSentinelDefeated"))

    exit_actor = spawn(exit_class, (0, 1850, 100), "CH19_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1425)
    set_property(exit_actor, "completion_currency_reward", 710)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter20/L_CH20_TheFinalHunter"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 320), "CH19_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(45.0, 45.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 19 with three origin seals and two origin sentinels.")
    unreal.log_warning("HUNTER: Build NavMesh and test interactions, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_19()
