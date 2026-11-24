"""Generate Chapter 15's Worlds Unsealed breach mission in Unreal Editor."""
import unreal

LEVEL_PATH = "/Game/Maps/Chapter15/L_CH15_WorldsUnsealed"


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


def build_chapter_15():
    unreal.EditorAssetLibrary.make_directory("/Game/Maps/Chapter15")
    if not unreal.EditorLevelLibrary.new_level(LEVEL_PATH):
        raise RuntimeError("Could not create " + LEVEL_PATH)

    arena_class = native_class("HunterPrototypeArenaActor")
    trigger_class = native_class("HunterQuestTriggerActor")
    monster_class = native_class("HunterMonsterBase")
    exit_class = native_class("HunterChapterExitActor")

    arena = spawn(arena_class, (0, 0, 0), "CH15_Converging_Breach_Arena")
    set_property(arena, "arena_half_width", 2200.0)
    set_property(arena, "arena_half_depth", 2150.0)
    set_property(arena, "wall_height", 900.0)
    spawn(unreal.PlayerStart, (0, -1700, 110), "CH15_PlayerStart",
          unreal.Rotator(0.0, 90.0, 0.0))

    sun = spawn(unreal.DirectionalLight, (0, 0, 1200), "CH15_Breach_Light",
                unreal.Rotator(-48.0, -24.0, 0.0))
    set_property(sun, "intensity", 4.8)
    sky = spawn(unreal.SkyLight, (0, 0, 850), "CH15_Breach_Sky")
    set_property(sky, "intensity", 0.7)
    fog = spawn(unreal.ExponentialHeightFog, (0, 0, 0), "CH15_Dimensional_Fog")
    set_property(fog, "fog_density", 0.03)

    anchor_points = [(-1400, 300, 110), (-500, 1050, 110),
                     (500, 1050, 110), (1400, 300, 110)]
    for index, point in enumerate(anchor_points, start=1):
        anchor = spawn(trigger_class, point, "CH15_Breach_Anchor_%02d" % index)
        set_property(anchor, "quest_event_tag", unreal.Name("BreachAnchorStabilized"))
        set_property(anchor, "event_amount", 1)
        set_property(anchor, "trigger_once", True)
        set_property(anchor, "destroy_after_trigger", False)
        set_property(anchor, "requires_interaction", True)
        set_property(anchor, "interaction_prompt_text", unreal.Text("E  STABILIZE BREACH ANCHOR"))
        set_property(anchor, "completion_prompt_text", unreal.Text("BREACH ANCHOR STABILIZED"))

    guardian_points = [(-650, -50, 110), (650, -50, 110)]
    for index, point in enumerate(guardian_points, start=1):
        guardian = spawn(monster_class, point, "CH15_BREACH_GUARDIAN_%02d" % index)
        set_property(guardian, "defeat_quest_event_tag", unreal.Name("BreachGuardianDefeated"))

    exit_actor = spawn(exit_class, (0, 1780, 100), "CH15_Chapter_Exit")
    set_property(exit_actor, "completion_experience_reward", 1125)
    set_property(exit_actor, "completion_currency_reward", 560)
    set_property(exit_actor, "save_slot_name", "HunterCampaign")
    set_property(exit_actor, "next_chapter_map_name", unreal.Name("/Game/Maps/Chapter16/L_CH16_SevenBeacons"))

    nav = spawn(unreal.NavMeshBoundsVolume, (0, 0, 300), "CH15_NavMeshBounds")
    nav.set_actor_scale3d(unreal.Vector(44.0, 44.0, 7.0))
    if not unreal.EditorLevelLibrary.save_current_level():
        raise RuntimeError("Unreal Editor did not save " + LEVEL_PATH)
    unreal.log("HUNTER: Saved Chapter 15 with four stabilizable breach anchors and two guardians.")
    unreal.log_warning("HUNTER: Build NavMesh and test anchor interactions, enemy AI, quest gating and campaign travel in Unreal Editor.")


build_chapter_15()
