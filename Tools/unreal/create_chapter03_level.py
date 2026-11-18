"""Generate Chapter 3's guild evaluation arena in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
The level contains five native trial opponents and an E-key completion seal.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter03/L_CH03_GuildEvaluation"


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


def build_chapter_three():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter03")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    monster_class = native_class("HunterMonsterBase")
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH03_GuildEvaluation_Arena")
    set_property(arena, "arena_half_width", 1600.0)
    set_property(arena, "arena_half_depth", 1600.0)
    set_property(arena, "wall_height", 650.0)

    spawn(unreal.PlayerStart, (0, -1250, 110), "CH03_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1000), "CH03_Sun",
                unreal.Rotator(-48.0, -35.0, 0.0))
    set_property(sun, "intensity", 7.0)
    sky = spawn(unreal.SkyLight, (0, 0, 750), "CH03_SkyLight")
    set_property(sky, "intensity", 1.0)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH03_ArenaFog")
    set_property(fog, "fog_density", 0.012)

    # Placed native enemies inherit the engine blockout silhouette and AI pursuit.
    spawn_points = [(-650, -250, 110), (0, 150, 110), (650, -250, 110),
                    (-400, 700, 110), (400, 700, 110)]
    for index, point in enumerate(spawn_points, start=1):
        enemy = spawn(monster_class, point, "CH03_TrialOpponent_%02d" % index)
        set_property(enemy, "defeat_quest_event_tag",
                     unreal.Name("TrialOpponentDefeated"))

    seal = spawn(trigger_class, (0, 1180, 100), "CH03_Trial_Completion_Seal")
    set_property(seal, "quest_event_tag", unreal.Name("TrialSealActivated"))
    set_property(seal, "event_amount", 1)
    set_property(seal, "trigger_once", True)
    set_property(seal, "destroy_after_trigger", False)
    set_property(seal, "requires_interaction", True)
    set_property(seal, "interaction_prompt_text",
                 unreal.Text("E  ACTIVATE TRIAL SEAL"))
    set_property(seal, "completion_prompt_text", unreal.Text("GUILD EVALUATION PASSED"))

    exit_actor = spawn(exit_class, (0, 1450, 100), "CH03_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 225)
    set_property(exit_actor, "completion_currency_reward", 110)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 250), "CH03_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(34.0, 34.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 3 guild evaluation with five tagged opponents and an interaction seal.")
    unreal.log_warning("HUNTER: Build NavMesh and test enemy defeat events, seal gating and chapter exit in Unreal Editor.")


build_chapter_three()
