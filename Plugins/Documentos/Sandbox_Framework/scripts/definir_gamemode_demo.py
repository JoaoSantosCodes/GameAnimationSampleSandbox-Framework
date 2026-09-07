# Aponta o World Settings do nivel de demonstracao para o GameMode da demo, para que abrir o
# mapa e dar Play ja mostre o HUD com o painel de inventario.
#   UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<este arquivo>"
import unreal

DEMO = "/08_SandboxInventory/Demo"
LEVEL = DEMO + "/L_InventoryDemo"
GAMEMODE = DEMO + "/BP_DemoGameMode"


def log(msg):
    unreal.log("[gm] " + str(msg))


registro = unreal.AssetRegistryHelpers.get_asset_registry()
registro.scan_paths_synchronous([DEMO], force_rescan=True)
registro.wait_for_completion()

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
if not les.load_level(LEVEL):
    raise RuntimeError("nao consegui abrir " + LEVEL)
log("nivel aberto")

classe_gm = unreal.EditorAssetLibrary.load_blueprint_class(GAMEMODE)
if classe_gm is None:
    raise RuntimeError("GameMode nao resolvido: " + GAMEMODE)

mundo = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
ajustes = None
try:
    ajustes = mundo.get_world_settings()
except Exception:
    # Nem toda versao expoe get_world_settings; o WorldSettings tambem e um ator do nivel.
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for ator in eas.get_all_level_actors():
        if isinstance(ator, unreal.WorldSettings):
            ajustes = ator
            break

if ajustes is None:
    raise RuntimeError("World Settings nao encontrado")

ajustes.set_editor_property("default_game_mode", classe_gm)
log("GameMode do nivel = " + str(classe_gm.get_name()))

if not les.save_current_level():
    raise RuntimeError("save_current_level falhou")

# Conferir relendo, e nao pela chamada que acabou de ser feita.
confirmado = ajustes.get_editor_property("default_game_mode")
log("relido do World Settings: " + str(confirmado))
if confirmado is None:
    raise RuntimeError("GameMode nao persistiu")

log("OK")
