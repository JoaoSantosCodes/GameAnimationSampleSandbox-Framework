# Portao B - monta um projeto C++ vazio, copia so os plugins do framework e prova que
# eles compilam sem nada do repositorio original. E o teste que responde se ha produto.
import io, json, os, shutil, sys

ENGINE = r"D:\Unreal\Unreal Sistema\UE_5.8"
ENGINE_ID = "{A29EA5CD-4B0C-73B2-EF14-3E9E47C79842}"
SRC_REPO = r"D:\Unreal\GameAnimationSample"
DEST = r"D:\Unreal\PortaoB"  # caminho curto: o Windows aborta acao de build acima de 260 caracteres
NAME = "PortaoB"

PLUGINS = ["01_SandboxCommon", "02_SandboxInterfaces", "03_SandboxAssets", "04_SandboxCore",
           "05_SandboxCharacter", "06_SandboxCombat", "07_SandboxInteraction",
           "08_SandboxInventory", "09_SandboxUI", "10_SandboxDebug", "11_SandboxEditor"]

# Plugins de engine que os .uplugin do framework declaram (nenhum da Lyra deve sobrar aqui).
ENGINE_PLUGINS = ["ModularGameplay", "GameplayAbilities", "EnhancedInput", "PCG",
                  "SmartObjects", "StateTree", "GameplayStateTree"]

TARGET = '''using UnrealBuildTool;

public class {name}{suffix}Target : TargetRules
{{
	public {name}{suffix}Target(TargetInfo Target) : base(Target)
	{{
		Type = TargetType.{type};
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("{name}");
	}}
}}
'''

BUILD = '''using UnrealBuildTool;

public class {name} : ModuleRules
{{
	public {name}(ReadOnlyTargetRules Target) : base(Target)
	{{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {{ "Core", "CoreUObject", "Engine", "InputCore" }});
	}}
}}
'''

MODULE = '''#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, {name}, "{name}");
'''

IGNORE = shutil.ignore_patterns("Intermediate", "Binaries", "Saved", ".git")


def main():
    if os.path.isdir(DEST):
        shutil.rmtree(DEST)
    os.makedirs(os.path.join(DEST, "Source", NAME))
    os.makedirs(os.path.join(DEST, "Plugins"))
    os.makedirs(os.path.join(DEST, "Config"))

    for p in PLUGINS:
        shutil.copytree(os.path.join(SRC_REPO, "Plugins", p),
                        os.path.join(DEST, "Plugins", p), ignore=IGNORE)
    print("copiados %d plugins (sem Intermediate/Binaries)" % len(PLUGINS))

    uproject = {
        "FileVersion": 3,
        "EngineAssociation": ENGINE_ID,
        "Category": "",
        "Description": "Projeto vazio para provar que os plugins do framework sao autocontidos.",
        "Modules": [{"Name": NAME, "Type": "Runtime", "LoadingPhase": "Default"}],
        "Plugins": [{"Name": n, "Enabled": True} for n in ENGINE_PLUGINS + PLUGINS],
    }
    io.open(os.path.join(DEST, NAME + ".uproject"), "w", encoding="utf-8", newline="").write(
        json.dumps(uproject, indent="\t") + "\n")

    src = os.path.join(DEST, "Source")
    io.open(os.path.join(src, NAME + ".Target.cs"), "w", encoding="utf-8", newline="").write(
        TARGET.format(name=NAME, suffix="", type="Game"))
    io.open(os.path.join(src, NAME + "Editor.Target.cs"), "w", encoding="utf-8", newline="").write(
        TARGET.format(name=NAME, suffix="Editor", type="Editor"))
    io.open(os.path.join(src, NAME, NAME + ".Build.cs"), "w", encoding="utf-8", newline="").write(
        BUILD.format(name=NAME))
    io.open(os.path.join(src, NAME, NAME + ".cpp"), "w", encoding="utf-8", newline="").write(
        MODULE.format(name=NAME))

    # Config minimo: so o necessario para o editor abrir.
    # Abrir direto no mapa da demo: sem isto o editor sobe num nivel vazio e quem esta testando
    # precisa saber de cor onde o mapa mora.
    demo = "/08_SandboxInventory/Demo/L_InventoryDemo.L_InventoryDemo"
    io.open(os.path.join(DEST, "Config", "DefaultEngine.ini"), "w", encoding="utf-8", newline="").write(
        "[/Script/EngineSettings.GameMapsSettings]\n"
        "EditorStartupMap=%s\n"
        "GameDefaultMap=%s\n" % (demo, demo))
    io.open(os.path.join(DEST, "Config", "DefaultGame.ini"), "w", encoding="utf-8", newline="").write(
        "[/Script/EngineSettings.GeneralProjectSettings]\nProjectName=PortaoB\n")

    print("projeto montado em:", DEST)
    print("\nproximo passo:")
    print(r'  & "%s\Engine\Build\BatchFiles\Build.bat" %sEditor Win64 Development -Project="%s\%s.uproject" -WaitMutex'
          % (ENGINE, NAME, DEST, NAME))


if __name__ == "__main__":
    main()
