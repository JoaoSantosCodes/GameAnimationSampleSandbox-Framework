#include "Commands/UnrealMCPDataAssetCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/DataAsset.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorAssetLibrary.h"
#include "UObject/UnrealType.h"
#include "GameplayTagContainer.h"

FUnrealMCPDataAssetCommands::FUnrealMCPDataAssetCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPDataAssetCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandName == TEXT("create_data_asset"))
    {
        return HandleCreateDataAsset(Params);
    }
    if (CommandName == TEXT("set_data_asset_property"))
    {
        return HandleSetDataAssetProperty(Params);
    }
    if (CommandName == TEXT("add_instanced_object"))
    {
        return HandleAddInstancedObject(Params);
    }
    if (CommandName == TEXT("get_data_asset_properties"))
    {
        return HandleGetDataAssetProperties(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown Data Asset command: %s"), *CommandName));
}

namespace
{
    /** Carrega o asset por nome curto ou caminho completo, com mensagem util quando falha. */
    UDataAsset* LoadDataAsset(const TSharedPtr<FJsonObject>& Params, FString& OutPath, FString& OutError)
    {
        FString NameOrPath;
        if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("asset_path"), TEXT("asset_name"), TEXT("name")}, NameOrPath))
        {
            OutError = TEXT("Missing 'asset_path' parameter");
            return nullptr;
        }

        OutPath = FUnrealMCPCommonUtils::ResolveAssetPath(NameOrPath, TEXT("/Game/Data/"));
        UDataAsset* Asset = Cast<UDataAsset>(UEditorAssetLibrary::LoadAsset(OutPath));
        if (!Asset)
        {
            OutError = FString::Printf(TEXT("Data Asset nao encontrado: '%s' (resolvido para '%s')"), *NameOrPath, *OutPath);
        }
        return Asset;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPDataAssetCommands::HandleCreateDataAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetClassName;
    if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("asset_class"), TEXT("class_name"), TEXT("type")}, AssetClassName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_class' parameter"));
    }

    FString AssetName;
    if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("name"), TEXT("asset_name")}, AssetName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    FString PackagePath = TEXT("/Game/Data/");
    Params->TryGetStringField(TEXT("path"), PackagePath);
    if (!PackagePath.EndsWith(TEXT("/")))
    {
        PackagePath += TEXT("/");
    }

    UClass* AssetClass = FUnrealMCPCommonUtils::FindClassByNameOrPath(AssetClassName);
    if (!AssetClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("Classe nao encontrada: '%s'. Use nome curto (SBItemDefinition) ou caminho (/Script/SandboxInventory.SBItemDefinition)."),
            *AssetClassName));
    }

    if (!AssetClass->IsChildOf(UDataAsset::StaticClass()))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("'%s' nao deriva de UDataAsset."), *AssetClassName));
    }

    if (AssetClass->HasAnyClassFlags(CLASS_Abstract))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("'%s' e abstrata e nao pode ser instanciada."), *AssetClassName));
    }

    const FString FullPath = AssetName.StartsWith(TEXT("/")) ? AssetName : PackagePath + AssetName;
    if (UEditorAssetLibrary::DoesAssetExist(FullPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Asset ja existe: %s"), *FullPath));
    }

    UPackage* Package = CreatePackage(*FullPath);
    if (!Package)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Falha ao criar o pacote"));
    }

    // Pacote recem-criado nasce como "parcialmente carregado" e o editor se recusa a salva-lo.
    // Em /Game o sintoma nao aparece; num mount de plugin, o save falha com
    // "cannot be saved as it has only been partially loaded" e o asset so existe em memoria.
    Package->MarkAsFullyLoaded();

    FString ShortName = AssetName;
    if (ShortName.Contains(TEXT("/")))
    {
        ShortName.Split(TEXT("/"), nullptr, &ShortName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    }

    UDataAsset* NewAsset = NewObject<UDataAsset>(Package, AssetClass, *ShortName, RF_Public | RF_Standalone);
    if (!NewAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Falha ao criar o Data Asset"));
    }

    FAssetRegistryModule::AssetCreated(NewAsset);
    const bool bSaved = FUnrealMCPCommonUtils::SaveAssetToDisk(NewAsset);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("name"), ShortName);
    Result->SetStringField(TEXT("path"), FullPath);
    Result->SetStringField(TEXT("class"), AssetClass->GetName());
    Result->SetBoolField(TEXT("saved_to_disk"), bSaved);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataAssetCommands::HandleSetDataAssetProperty(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath, Error;
    UDataAsset* Asset = LoadDataAsset(Params, AssetPath, Error);
    if (!Asset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    FString PropertyName;
    if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("property_name"), TEXT("property")}, PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_name' parameter"));
    }

    if (!Params->HasField(TEXT("property_value")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_value' parameter"));
    }

    FString ErrorMessage;
    Asset->Modify();
    if (!FUnrealMCPCommonUtils::SetObjectProperty(Asset, PropertyName, Params->TryGetField(TEXT("property_value")), ErrorMessage))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
    }

    const bool bSaved = FUnrealMCPCommonUtils::SaveAssetToDisk(Asset);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetStringField(TEXT("property"), PropertyName);
    Result->SetBoolField(TEXT("saved_to_disk"), bSaved);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataAssetCommands::HandleAddInstancedObject(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath, Error;
    UDataAsset* Asset = LoadDataAsset(Params, AssetPath, Error);
    if (!Asset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    FString ArrayPropertyName;
    if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("array_property"), TEXT("property_name")}, ArrayPropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'array_property' parameter"));
    }

    FString ObjectClassName;
    if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("object_class"), TEXT("class_name")}, ObjectClassName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'object_class' parameter"));
    }

    FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(Asset->GetClass(), *ArrayPropertyName);
    if (!ArrayProperty)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("'%s' nao e um array em %s"), *ArrayPropertyName, *Asset->GetClass()->GetName()));
    }

    FObjectProperty* InnerObjectProperty = CastField<FObjectProperty>(ArrayProperty->Inner);
    if (!InnerObjectProperty)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("O array '%s' nao guarda objetos."), *ArrayPropertyName));
    }

    UClass* ObjectClass = FUnrealMCPCommonUtils::FindClassByNameOrPath(ObjectClassName);
    if (!ObjectClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Classe nao encontrada: '%s'"), *ObjectClassName));
    }

    if (ObjectClass->HasAnyClassFlags(CLASS_Abstract))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("'%s' e abstrata. Use uma subclasse concreta."), *ObjectClassName));
    }

    if (!ObjectClass->IsChildOf(InnerObjectProperty->PropertyClass))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
            TEXT("'%s' nao deriva de '%s', exigida pelo array '%s'."),
            *ObjectClassName, *InnerObjectProperty->PropertyClass->GetName(), *ArrayPropertyName));
    }

    // O sub-objeto precisa ter o proprio asset como outer: e o que torna a instancia parte do
    // asset e nao uma referencia externa que se perde ao salvar.
    Asset->Modify();
    UObject* NewInstance = NewObject<UObject>(Asset, ObjectClass, NAME_None, RF_Public);
    if (!NewInstance)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Falha ao instanciar o sub-objeto"));
    }

    // Propriedades opcionais do sub-objeto, aplicadas antes de inserir no array.
    TArray<FString> PropertiesSet;
    TArray<FString> PropertiesFailed;
    const TSharedPtr<FJsonObject>* SubProps = nullptr;
    if (Params->TryGetObjectField(TEXT("properties"), SubProps) && SubProps && SubProps->IsValid())
    {
        for (const auto& Pair : (*SubProps)->Values)
        {
            const FString PropName(Pair.Key);
            const TSharedPtr<FJsonValue> PropValue = Pair.Value;

            FString SubError;
            if (FUnrealMCPCommonUtils::SetObjectProperty(NewInstance, PropName, PropValue, SubError))
            {
                PropertiesSet.Add(PropName);
            }
            else
            {
                PropertiesFailed.Add(FString::Printf(TEXT("%s (%s)"), *PropName, *SubError));
            }
        }
    }

    FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(Asset));
    const int32 Index = ArrayHelper.AddValue();
    InnerObjectProperty->SetObjectPropertyValue(ArrayHelper.GetRawPtr(Index), NewInstance);

    const bool bSaved = FUnrealMCPCommonUtils::SaveAssetToDisk(Asset);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetStringField(TEXT("array_property"), ArrayPropertyName);
    Result->SetStringField(TEXT("object_class"), ObjectClass->GetName());
    Result->SetNumberField(TEXT("index"), Index);
    Result->SetNumberField(TEXT("array_size"), ArrayHelper.Num());
    Result->SetBoolField(TEXT("saved_to_disk"), bSaved);

    TArray<TSharedPtr<FJsonValue>> SetArray;
    for (const FString& Name : PropertiesSet)
    {
        SetArray.Add(MakeShared<FJsonValueString>(Name));
    }
    Result->SetArrayField(TEXT("properties_set"), SetArray);

    // Falha de propriedade nao invalida a insercao, mas precisa aparecer: sub-objeto inserido
    // com campo nao aplicado e defeito silencioso.
    if (PropertiesFailed.Num() > 0)
    {
        TArray<TSharedPtr<FJsonValue>> FailedArray;
        for (const FString& Name : PropertiesFailed)
        {
            FailedArray.Add(MakeShared<FJsonValueString>(Name));
        }
        Result->SetArrayField(TEXT("properties_failed"), FailedArray);
    }

    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataAssetCommands::HandleGetDataAssetProperties(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath, Error;
    UDataAsset* Asset = LoadDataAsset(Params, AssetPath, Error);
    if (!Asset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetStringField(TEXT("class"), Asset->GetClass()->GetName());

    TSharedPtr<FJsonObject> Props = MakeShared<FJsonObject>();
    for (TFieldIterator<FProperty> It(Asset->GetClass()); It; ++It)
    {
        FProperty* Property = *It;
        const FString Name = Property->GetName();
        const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Asset);

        if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
        {
            FScriptArrayHelper Helper(ArrayProp, ValuePtr);
            TArray<TSharedPtr<FJsonValue>> Entries;
            for (int32 i = 0; i < Helper.Num(); ++i)
            {
                if (const FObjectProperty* InnerObj = CastField<FObjectProperty>(ArrayProp->Inner))
                {
                    const UObject* Element = InnerObj->GetObjectPropertyValue(Helper.GetRawPtr(i));
                    Entries.Add(MakeShared<FJsonValueString>(Element ? Element->GetClass()->GetName() : TEXT("null")));
                }
                else
                {
                    FString Exported;
                    ArrayProp->Inner->ExportTextItem_Direct(Exported, Helper.GetRawPtr(i), nullptr, nullptr, PPF_None);
                    Entries.Add(MakeShared<FJsonValueString>(Exported));
                }
            }
            Props->SetArrayField(Name, Entries);
            continue;
        }

        FString Exported;
        Property->ExportTextItem_Direct(Exported, ValuePtr, nullptr, nullptr, PPF_None);
        Props->SetStringField(Name, Exported);
    }

    Result->SetObjectField(TEXT("properties"), Props);
    return Result;
}
