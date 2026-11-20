"""Generate Chapter 8's Hunter Reborn greybox level in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter08/L_CH08_HunterReborn"


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


def build_chapter_8():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter08")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    monster_class = native_class("HunterMonsterBase")
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH08_Hunter_Reborn")
    set_property(arena, "arena_half_width", 1850.0)
    set_property(arena, "arena_half_depth", 1850.0)
    set_property(arena, "wall_height", 690.0)

    spawn(unreal.PlayerStart, (0, -1450, 110), "CH08_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1050), "CH08_Sun",
                unreal.Rotator(-44.0, -30.0, 0.0))
    set_property(sun, "intensity", 5.2)
    sky = spawn(unreal.SkyLight, (0, 0, 780), "CH08_SkyLight")
    set_property(sky, "intensity", 0.85)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH08_Atmosphere")
    set_property(fog, "fog_density", 0.016)

    spawn_points = [(-900, -300, 110), (0, -100, 110), (900, -300, 110),
                    (-750, 600, 110), (0, 800, 110), (750, 600, 110)]
    for index, point in enumerate(spawn_points, start=1):
        enemy = spawn(monster_class, point, "CH08_Awakening_Ambusher_%02d" % index)
        set_property(enemy, "defeat_quest_event_tag", unreal.Name("AwakeningCreatureDefeated"))

    altar = spawn(trigger_class, (0, 1180, 100), "CH08_Awakening_Altar")
    set_property(altar, "quest_event_tag", unreal.Name("AwakeningAltarActivated"))
    set_property(altar, "event_amount", 1)
    set_property(altar, "trigger_once", True)
    set_property(altar, "destroy_after_trigger", False)
    set_property(altar, "requires_interaction", True)
    set_property(altar, "interaction_prompt_text", unreal.Text("E  ACTIVATE AWAKENING ALTAR"))
    set_property(altar, "completion_prompt_text", unreal.Text("AWAKENING COMPLETE"))
    
    exit_actor = spawn(exit_class, (0, 1580, 100), "CH08_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 600)
    set_property(exit_actor, "completion_currency_reward", 300)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH08_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(39.0, 39.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 8 Hunter Reborn with configured campaign objective sources.")
    unreal.log_warning("HUNTER: Build NavMesh and test AI, interactions, quest gating and campaign exit in Unreal Editor.")


build_chapter_8()
