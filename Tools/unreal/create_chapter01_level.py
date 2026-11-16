"""Generate the Chapter 1 playable greybox in Unreal Editor.

Run via Tools > Execute Python Script after compiling the HunterGame C++ module.
Creates a native level asset with the Chapter 1 campaign's actual three-enemy
breach/reward flow. Art, UI, animation and packaged build still require authoring.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter01/L_CH01_FirstBreach"


def native_class(name):
    result = unreal.load_class(None, "/Script/HunterGame." + name)
    if not result:
        raise RuntimeError("Missing native class " + name + "; compile HunterGame first.")
    return result


def spawn(actor_class, location, label, rotation=None):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        actor_class, unreal.Vector(*location),
        rotation or unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError("Could not spawn " + label)
    actor.set_actor_label(label)
    return actor


def set_property(actor, name, value):
    try:
        actor.set_editor_property(name, value)
    except Exception as error:
        raise RuntimeError("Could not configure " + name + " on " +
                           actor.get_name() + ": " + str(error))


def make_spawn_transform(location):
    transform = unreal.Transform()
    transform.set_editor_property("translation", unreal.Vector(*location))
    transform.set_editor_property("scale3d", unreal.Vector(1.0, 1.0, 1.0))
    return transform


def build_chapter_one():
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    native_class("ArinValeCharacter")  # AHunterGameMode uses this as DefaultPawnClass.
    monster_class = native_class("HunterMonsterBase")
    breach_class = native_class("HunterBreachEncounterActor")
    pickup_class = native_class("HunterPickupActor")
    checkpoint_class = native_class("HunterCheckpointActor")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH01_Arena_Greybox")
    set_property(arena, "arena_half_width", 1800.0)
    set_property(arena, "arena_half_depth", 1800.0)
    set_property(arena, "wall_height", 600.0)

    # The project's native game mode spawns Arin at this PlayerStart.
    spawn(unreal.PlayerStart, (0, -1450, 110), "CH01_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    # The real Chapter 1 encounter emits three MonsterDefeated events, then
    # spawns a BreachShard pickup that emits BreachShardCollected on collection.
    encounter = spawn(breach_class, (0, 0, 0), "CH01_FirstBreach_Encounter")
    set_property(encounter, "monster_class", monster_class)
    set_property(encounter, "monster_spawn_points", [
        make_spawn_transform((-520, 350, 110)),
        make_spawn_transform((0, 650, 110)),
        make_spawn_transform((520, 350, 110)),
    ])
    set_property(encounter, "fallback_monster_count", 3)
    set_property(encounter, "spawn_interval_seconds", 1.25)
    set_property(encounter, "reward_pickup_class", pickup_class)
    reward = unreal.HunterInventoryItem()
    reward.set_editor_property("item_id", unreal.Name("BreachShard"))
    reward.set_editor_property("display_name", unreal.Text("Breach Shard"))
    reward.set_editor_property("item_type", unreal.HunterItemType.QUEST)
    reward.set_editor_property("max_stack", 1)
    reward.set_editor_property("quantity", 1)
    set_property(encounter, "reward_item", reward)
    set_property(encounter, "reward_quest_event_tag",
                 unreal.Name("BreachShardCollected"))
    set_property(encounter, "trigger_radius", 350.0)

    checkpoint = spawn(checkpoint_class, (0, -1050, 100),
                       "CH01_Entry_Checkpoint")
    set_property(checkpoint, "checkpoint_id", unreal.Name("CH01_Entry"))
    set_property(checkpoint, "save_slot_name", "HunterCampaign")

    chapter_exit = spawn(exit_class, (0, 1500, 100),
                         "CH01_Chapter_Exit")
    set_property(chapter_exit, "completion_experience_reward", 100)
    set_property(chapter_exit, "completion_currency_reward", 50)

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 250),
                "CH01_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(38.0, 38.0, 7.0))

    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 1 greybox with native campaign encounter and reward flow.")
    unreal.log_warning("HUNTER: Open the level, press P to inspect NavMesh, then Play-test AI, checkpoint, reward collection and chapter exit. This script has not been runtime-tested outside Unreal Editor.")


build_chapter_one()
