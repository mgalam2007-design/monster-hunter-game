"""Create a runnable Chapter 1 greybox level from the committed native C++ actors.

Run inside Unreal Editor: Tools > Execute Python Script.
Requires the HunterGame C++ module to have been compiled by the local Unreal installation.
This creates a level asset; it does not create final art, UI, animations, or a packaged build.
"""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter01/L_CH01_FirstBreach"


def load_native_class(class_name):
    loaded = unreal.load_class(None, "/Script/HunterGame." + class_name)
    if not loaded:
        raise RuntimeError("Could not load /Script/HunterGame." + class_name +
                           ". Compile the HunterGame module first.")
    return loaded


def spawn(editor_library, actor_class, location, label, rotation=None):
    actor = editor_library.spawn_actor_from_class(
        actor_class,
        unreal.Vector(location[0], location[1], location[2]),
        rotation or unreal.Rotator(0.0, 0.0, 0.0))
    if not actor:
        raise RuntimeError("Failed to spawn " + label)
    actor.set_actor_label(label)
    return actor


def configure_if_present(actor, property_name, value):
    try:
        actor.set_editor_property(property_name, value)
    except Exception:
        unreal.log_warning("Could not set optional property " + property_name +
                           " on " + actor.get_name())


def build_chapter_one():
    editor = unreal.EditorLevelLibrary
    if not editor.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create level " + LEVEL_PATH)

    arena_class = load_native_class("HunterPrototypeArenaActor")
    load_native_class("ArinValeCharacter")  # Fail early if the default pawn class is unavailable.
    monster_class = load_native_class("HunterMonsterBase")
    checkpoint_class = load_native_class("HunterCheckpointActor")
    exit_class = load_native_class("HunterChapterExitActor")

    arena = spawn(editor, arena_class, (0, 0, 0), "CH01_Arena_Greybox")
    configure_if_present(arena, "arena_half_width", 1800.0)
    configure_if_present(arena, "arena_half_depth", 1800.0)
    configure_if_present(arena, "wall_height", 600.0)

    player_start = spawn(editor, unreal.PlayerStart, (0, -1450, 110),
                         "CH01_PlayerStart", unreal.Rotator(0, 0, 90))
    # AHunterGameMode already spawns AArinValeCharacter as its default pawn.
    # Keep one PlayerStart so the game mode creates exactly one player pawn.

    # Three independent native enemies make the greybox immediately testable.
    enemy_positions = [(-500, 250, 110), (0, 650, 110), (500, 250, 110)]
    for index, position in enumerate(enemy_positions):
        enemy = spawn(editor, monster_class, position,
                      "CH01_BreachStalker_" + str(index + 1))
        configure_if_present(enemy, "max_health", 120.0)
        configure_if_present(enemy, "detection_radius", 1400.0)
        configure_if_present(enemy, "attack_range", 180.0)
        configure_if_present(enemy, "attack_damage", 12.0)
        configure_if_present(enemy, "attack_cooldown", 1.6)
        configure_if_present(enemy, "attack_windup_duration", 0.45)
        configure_if_present(enemy, "chase_speed", 430.0)
        configure_if_present(enemy, "experience_reward", 25)
        configure_if_present(enemy, "currency_reward", 5)
        configure_if_present(enemy, "auto_possess_ai",
                             unreal.AutoPossessAI.PLACED_IN_WORLD_OR_SPAWNED)

    checkpoint = spawn(editor, checkpoint_class, (0, -1050, 100),
                       "CH01_Entry_Checkpoint")
    configure_if_present(checkpoint, "checkpoint_id",
                         unreal.Name("CH01_Entry"))
    configure_if_present(checkpoint, "save_slot_name", "HunterCampaign")

    chapter_exit = spawn(editor, exit_class, (0, 1500, 100),
                         "CH01_Chapter_Exit")
    configure_if_present(chapter_exit, "completion_experience_reward", 100)
    configure_if_present(chapter_exit, "completion_currency_reward", 50)

    nav_bounds = spawn(editor, unreal.NavMeshBoundsVolume, (0, 0, 250),
                       "CH01_NavMeshBounds")
    nav_bounds.set_actor_scale3d(unreal.Vector(38.0, 38.0, 7.0))

    # Keep the native game mode and generated actors visible in the World Outliner.
    editor.save_current_level()
    unreal.log("HUNTER: Created " + LEVEL_PATH)
    unreal.log("HUNTER: Added arena, player start, Arin, three native enemies, checkpoint, exit and NavMesh bounds.")
    unreal.log_warning("HUNTER: Build and open the level in Unreal Editor, press P to inspect NavMesh, and verify AI navigation. UI, quest reward wiring, authored art and animation remain to be configured.")


build_chapter_one()
