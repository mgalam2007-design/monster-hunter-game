"""Generate Chapter 5's corrupted mining-settlement contract in Unreal Editor.

Compile HunterGame, then run this script from Tools > Execute Python Script.
The greybox includes four tagged creatures, three E-key ore recovery points and a gated exit.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter05/L_CH05_MiningSettlement"


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


def build_chapter_five():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter05")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    monster_class = native_class("HunterMonsterBase")
    trigger_class = native_class("HunterQuestTriggerActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH05_Corrupted_Mining_Settlement")
    set_property(arena, "arena_half_width", 1900.0)
    set_property(arena, "arena_half_depth", 1900.0)
    set_property(arena, "wall_height", 680.0)

    spawn(unreal.PlayerStart, (0, -1500, 110), "CH05_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1050), "CH05_Sun",
                unreal.Rotator(-42.0, -32.0, 0.0))
    set_property(sun, "intensity", 4.8)
    sky = spawn(unreal.SkyLight, (0, 0, 760), "CH05_SkyLight")
    set_property(sky, "intensity", 0.75)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH05_Mine_Dust")
    set_property(fog, "fog_density", 0.018)

    # Four native AI enemies report the Chapter 5 contract defeat tag.
    spawn_points = [(-700, -350, 110), (700, -250, 110),
                    (-450, 550, 110), (500, 700, 110)]
    for index, point in enumerate(spawn_points, start=1):
        enemy = spawn(monster_class, point, "CH05_Mining_Creature_%02d" % index)
        set_property(enemy, "defeat_quest_event_tag",
                     unreal.Name("MiningCreatureDefeated"))

    # Each sample point requires E, so collection is deliberate rather than
    # an accidental overlap. The trigger reports the Collect objective tag.
    sample_points = [(-900, 1050, 100), (0, 1180, 100), (900, 1050, 100)]
    for index, point in enumerate(sample_points, start=1):
        sample = spawn(trigger_class, point, "CH05_Corrupted_Ore_Sample_%02d" % index)
        set_property(sample, "quest_event_tag", unreal.Name("CorruptedOreCollected"))
        set_property(sample, "event_amount", 1)
        set_property(sample, "trigger_once", True)
        set_property(sample, "destroy_after_trigger", False)
        set_property(sample, "requires_interaction", True)
        set_property(sample, "interaction_prompt_text",
                     unreal.Text("E  RECOVER ORE SAMPLE"))
        set_property(sample, "completion_prompt_text",
                     unreal.Text("CORRUPTED ORE RECOVERED"))

    exit_actor = spawn(exit_class, (0, 1600, 100), "CH05_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 375)
    set_property(exit_actor, "completion_currency_reward", 185)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter06/L_CH06_TrialOfSteel"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 260), "CH05_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(40.0, 40.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 5 mining settlement with four tagged enemies and three interactive ore samples.")
    unreal.log_warning("HUNTER: Build NavMesh and test enemy AI, ore interactions, quest gating and chapter exit in Unreal Editor.")


build_chapter_five()
