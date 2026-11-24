"""Generate Chapter 17's Last Sanctuary evacuation mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter17/L_CH17_LastSanctuary"


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


def build_chapter_17():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter17")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH17_Last_Sanctuary_Shelter")
    set_property(arena, "arena_half_width", 2250.0)
    set_property(arena, "arena_half_depth", 2200.0)
    set_property(arena, "wall_height", 920.0)
    spawn(unreal.PlayerStart, (0, -1750, 110), "CH17_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1250), "CH17_Sanctuary_Light",
                unreal.Rotator(-38.0, -28.0, 0.0))
    set_property(sun, "intensity", 4.5)
    sky = spawn(unreal.SkyLight, (0, 0, 850), "CH17_Sanctuary_Sky")
    set_property(sky, "intensity", 0.82)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH17_Evacuation_Smoke")
    set_property(fog, "fog_density", 0.022)

    evacuation_points = [(-1450, -300, 100), (-800, 650, 100), (0, 1050, 100),
                         (800, 650, 100), (1450, -300, 100)]
    for index, point in enumerate(evacuation_points, start=1):
        group = spawn(trigger_class, point, "CH17_Evacuation_Group_%02d" % index)
        set_property(group, "quest_event_tag", unreal.Name("SanctuaryGroupEvacuated"))
        set_property(group, "event_amount", 1)
        set_property(group, "trigger_once", True)
        set_property(group, "destroy_after_trigger", False)
        set_property(group, "requires_interaction", True)
        set_property(group, "interaction_prompt_text", unreal.Text("E  ESCORT CIVILIANS TO SAFETY"))
        set_property(group, "completion_prompt_text", unreal.Text("CIVILIANS EVACUATED"))

    invader_points = [(-1500, 800, 110), (-900, 1200, 110), (-300, 1450, 110),
                      (300, 1450, 110), (900, 1200, 110), (1500, 800, 110)]
    for index, point in enumerate(invader_points, start=1):
        invader = spawn(monster_class, point, "CH17_Sanctuary_Invader_%02d" % index)
        set_property(invader, "defeat_quest_event_tag", unreal.Name("SanctuaryInvaderDefeated"))

    exit_actor = spawn(exit_class, (0, 1800, 100), "CH17_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1275)
    set_property(exit_actor, "completion_currency_reward", 635)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter18/L_CH18_CrownOfRuin"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 300), "CH17_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(45.0, 45.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 17 with five evacuation groups and six sanctuary invaders.")
    unreal.log_warning("HUNTER: Build NavMesh and test interactions, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_17()
