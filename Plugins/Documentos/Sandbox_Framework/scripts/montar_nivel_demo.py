# Monta o nivel de demonstracao do inventario DENTRO do plugin.
# Roda como commandlet:
#   UnrealEditor-Cmd.exe <uproject> -run=pythonscript -script="<este arquivo>"
import unreal

LEVEL = "/08_SandboxInventory/Demo/L_InventoryDemo"
DEMO = "/08_SandboxInventory/Demo"

falhas = []


def log(msg):
    unreal.log("[demo] " + str(msg))


def erro(msg):
    falhas.append(str(msg))
    unreal.log_error("[demo] " + str(msg))


les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# No commandlet o Asset Registry nao varre a pasta do plugin sozinho: sem isto, os Blueprints
# existem em disco e mesmo assim o load falha com "could not be found in the Asset Registry".
registro = unreal.AssetRegistryHelpers.get_asset_registry()
registro.scan_paths_synchronous([DEMO], force_rescan=True)
registro.wait_for_completion()
log("registro varrido em " + DEMO)

if not les.new_level(LEVEL):
    raise RuntimeError("new_level falhou para " + LEVEL)
log("nivel criado: " + LEVEL)

cubo = unreal.EditorAssetLibrary.load_asset("/Engine/BasicShapes/Cube")

# --- cenario minimo: chao, sol, ceu e ponto de spawn ------------------------
chao = eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(0, 0, -25))
chao.set_actor_label("Floor")
chao.static_mesh_component.set_static_mesh(cubo)
chao.set_actor_scale3d(unreal.Vector(30.0, 30.0, 0.5))

sol = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 800))
sol.set_actor_label("Sun")
sol.set_actor_rotation(unreal.Rotator(-50.0, -35.0, 0.0), False)

ceu = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 800))
ceu.set_actor_label("SkyLight")

inicio = eas.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-400, 0, 120))
inicio.set_actor_label("PlayerStart")

log("cenario base pronto")


# --- atores do inventario, vindos dos Blueprints de exemplo -----------------
def por_blueprint(nome, x, y, z, rotulo):
    caminho = DEMO + "/" + nome
    bp = unreal.EditorAssetLibrary.load_asset(caminho)
    if bp is None:
        erro("Blueprint nao encontrado: " + caminho)
        return None

    # spawn_actor_from_object devolve None para Blueprint; e preciso a classe gerada.
    classe = unreal.EditorAssetLibrary.load_blueprint_class(caminho)
    if classe is None:
        classe = bp.generated_class()
    if classe is None:
        erro("classe gerada nao resolvida para " + nome)
        return None

    ator = eas.spawn_actor_from_class(classe, unreal.Vector(x, y, z))
    if ator is None:
        erro("falha ao instanciar " + nome)
        return None
    ator.set_actor_label(rotulo)
    log("colocado " + rotulo + " a partir de " + nome)
    return ator


por_blueprint("BP_ScrapNode", 400.0, -300.0, 0.0, "ScrapNode")
por_blueprint("BP_Forge", 400.0, 300.0, 0.0, "Forge")
por_blueprint("BP_SupplyChest", 0.0, 350.0, 0.0, "SupplyChest")
por_blueprint("BP_TorchDrop", 150.0, 0.0, 60.0, "TorchDrop")

# --- salvar e conferir ------------------------------------------------------
if not les.save_current_level():
    raise RuntimeError("save_current_level falhou")

if not unreal.EditorAssetLibrary.does_asset_exist(LEVEL):
    raise RuntimeError("nivel nao existe no registro depois de salvar: " + LEVEL)

atores = eas.get_all_level_actors()
log("nivel salvo com %d atores" % len(atores))
for a in atores:
    log("   - %s (%s)" % (a.get_actor_label(), a.get_class().get_name()))

if falhas:
    raise RuntimeError("terminou com %d falha(s): %s" % (len(falhas), "; ".join(falhas)))

log("OK")
