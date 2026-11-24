"""Generate Chapter 18's Crown of Ruin throne-city mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter18/L_CH18_CrownOfRuin"


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


def build_chapter_18():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter18")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH18_Ruined_Throne_City")
    set_property(arena, "arena_half_width", 2200.0)
    set_property(arena, "arena_half_depth", 2200.0)
    set_property(arena, "wall_height", 940.0)
    spawn(unreal.PlayerStart, (0, -1750, 110), "CH18_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1250), "CH18_Ruin_Light",
                unreal.Rotator(-45.0, -24.0, 0.0))
    set_property(sun, "intensity", 4.2)
    sky = spawn(unreal.SkyLight, (0, 0, 850), "CH18_Ruin_Sky")
    set_property(sky, "intensity", 0.68)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH18_Throne_Ash")
    set_property(fog, "fog_density", 0.029)

    warden_points = [(-1350, 350, 110), (-450, 850, 110),
                     (450, 850, 110), (1350, 350, 110)]
    for index, point in enumerate(warden_points, start=1):
        warden = spawn(monster_class, point, "CH18_Throne_Warden_%02d" % index)
        set_property(warden, "defeat_quest_event_tag", unreal.Name("ThroneWardenDefeated"))

    core_points = [(-1100, 1250, 100), (0, 1450, 100), (1100, 1250, 100)]
    for index, point in enumerate(core_points, start=1):
        core = spawn(trigger_class, point, "CH18_Ruin_Command_Core_%02d" % index)
        set_property(core, "quest_event_tag", unreal.Name("RuinCommandCoreDestroyed"))
        set_property(core, "event_amount", 1)
        set_property(core, "trigger_once", True)
        set_property(core, "destroy_after_trigger", False)
        set_property(core, "requires_interaction", True)
        set_property(core, "interaction_prompt_text", unreal.Text("E  DESTROY COMMAND CORE"))
        set_property(core, "completion_prompt_text", unreal.Text("COMMAND CORE DESTROYED"))

    exit_actor = spawn(exit_class, (0, 1800, 100), "CH18_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1350)
    set_property(exit_actor, "completion_currency_reward", 675)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter19/L_CH19_BeyondTheGate"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 300), "CH18_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(44.0, 44.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 18 with four throne wardens and three command cores.")
    unreal.log_warning("HUNTER: Build NavMesh and test enemy AI, core interactions, quest gating and campaign travel in Unreal Editor.")


build_chapter_18()
