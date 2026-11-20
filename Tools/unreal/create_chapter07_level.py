"""Generate Chapter 7's Blue Mark greybox level in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter07/L_CH07_BlueMark"


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


def build_chapter_7():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter07")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH07_Blue_Mark")
    set_property(arena, "arena_half_width", 1850.0)
    set_property(arena, "arena_half_depth", 1850.0)
    set_property(arena, "wall_height", 690.0)

    spawn(unreal.PlayerStart, (0, -1450, 110), "CH07_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1050), "CH07_Sun",
                unreal.Rotator(-44.0, -30.0, 0.0))
    set_property(sun, "intensity", 5.2)
    sky = spawn(unreal.SkyLight, (0, 0, 780), "CH07_SkyLight")
    set_property(sky, "intensity", 0.85)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH07_Atmosphere")
    set_property(fog, "fog_density", 0.016)

    source = spawn(trigger_class, (0, -350, 100), "CH07_Blue_Mark_Source")
    set_property(source, "quest_event_tag", unreal.Name("BlueMarkSourceLocated"))
    set_property(source, "event_amount", 1)
    set_property(source, "trigger_once", True)
    set_property(source, "destroy_after_trigger", False)
    set_property(source, "requires_interaction", False)

    fragment_points = [(-850, 900, 100), (0, 1150, 100), (850, 900, 100)]
    for index, point in enumerate(fragment_points, start=1):
        fragment = spawn(trigger_class, point, "CH07_Resonance_Fragment_%02d" % index)
        set_property(fragment, "quest_event_tag", unreal.Name("ResonanceFragmentCollected"))
        set_property(fragment, "event_amount", 1)
        set_property(fragment, "trigger_once", True)
        set_property(fragment, "destroy_after_trigger", False)
        set_property(fragment, "requires_interaction", True)
        set_property(fragment, "interaction_prompt_text", unreal.Text("E  COLLECT RESONANCE"))
        set_property(fragment, "completion_prompt_text", unreal.Text("RESONANCE FRAGMENT SECURED"))
    
    exit_actor = spawn(exit_class, (0, 1580, 100), "CH07_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 525)
    set_property(exit_actor, "completion_currency_reward", 255)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter08/L_CH08_HunterReborn"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH07_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(39.0, 39.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 7 Blue Mark with configured campaign objective sources.")
    unreal.log_warning("HUNTER: Build NavMesh and test AI, interactions, quest gating and campaign exit in Unreal Editor.")


build_chapter_7()
