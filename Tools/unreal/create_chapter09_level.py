"""Generate Chapter 9's vanished-hunter investigation in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter09/L_CH09_VanishedHunters"


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


def build_chapter_9():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter09")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    monster_class = native_class("HunterMonsterBase")
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH09_Vanished_Hunters_Zone")
    set_property(arena, "arena_half_width", 1900.0)
    set_property(arena, "arena_half_depth", 1900.0)
    set_property(arena, "wall_height", 700.0)
    spawn(unreal.PlayerStart, (0, -1500, 110), "CH09_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1000), "CH09_Sun",
                unreal.Rotator(-48.0, -32.0, 0.0))
    set_property(sun, "intensity", 4.6)
    sky = spawn(unreal.SkyLight, (0, 0, 760), "CH09_SkyLight")
    set_property(sky, "intensity", 0.8)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH09_Signal_Haze")
    set_property(fog, "fog_density", 0.02)

    signal = spawn(trigger_class, (0, -250, 100), "CH09_Missing_Hunter_Signal")
    set_property(signal, "quest_event_tag", unreal.Name("MissingHunterSignalLocated"))
    set_property(signal, "event_amount", 1)
    set_property(signal, "trigger_once", True)
    set_property(signal, "destroy_after_trigger", False)
    set_property(signal, "requires_interaction", False)

    spawn_points = [(-800, -100, 110), (800, 0, 110),
                    (-550, 650, 110), (550, 700, 110)]
    for index, point in enumerate(spawn_points, start=1):
        enemy = spawn(monster_class, point, "CH09_Gate_Stalker_%02d" % index)
        set_property(enemy, "defeat_quest_event_tag", unreal.Name("GateStalkerDefeated"))

    recorder = spawn(trigger_class, (0, 1180, 100), "CH09_Expedition_Recorder")
    set_property(recorder, "quest_event_tag", unreal.Name("ExpeditionRecorderRecovered"))
    set_property(recorder, "event_amount", 1)
    set_property(recorder, "trigger_once", True)
    set_property(recorder, "destroy_after_trigger", False)
    set_property(recorder, "requires_interaction", True)
    set_property(recorder, "interaction_prompt_text", unreal.Text("E  RECOVER EXPEDITION RECORDER"))
    set_property(recorder, "completion_prompt_text", unreal.Text("FINAL SIGNAL RECORD RECOVERED"))

    exit_actor = spawn(exit_class, (0, 1580, 100), "CH09_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 675)
    set_property(exit_actor, "completion_currency_reward", 330)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter10/L_CH10_HollowDistrict"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH09_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(40.0, 40.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 9 with signal discovery, four stalkers and recorder recovery.")
    unreal.log_warning("HUNTER: Build NavMesh and test AI, quest events, recorder interaction and exit travel in Unreal Editor.")


build_chapter_9()
