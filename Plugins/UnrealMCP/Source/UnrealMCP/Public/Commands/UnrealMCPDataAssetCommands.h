#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Comandos de Data Asset.
 *
 * Existem porque o plugin nao tinha nenhum: o dispatch aceitava 36 comandos e nenhum criava
 * asset que nao fosse Blueprint ou Widget. Neste projeto a composicao de personagem passa
 * inteiramente por Data Asset (PawnData, ComponentSet), e sete tipos do framework — item,
 * receita, loot, quest, efeitos de superficie, camadas de animacao, configs — tinham zero
 * instancias por nao haver como cria-las automaticamente.
 *
 * Sao deliberadamente genericos: funcionam para qualquer UDataAsset, nao so para os do Sandbox.
 */
class UNREALMCP_API FUnrealMCPDataAssetCommands
{
public:
    FUnrealMCPDataAssetCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params);

private:
    /** create_data_asset: cria um UDataAsset de qualquer classe e grava em disco. */
    TSharedPtr<FJsonObject> HandleCreateDataAsset(const TSharedPtr<FJsonObject>& Params);

    /** set_data_asset_property: escreve uma propriedade simples e regrava. */
    TSharedPtr<FJsonObject> HandleSetDataAssetProperty(const TSharedPtr<FJsonObject>& Params);

    /**
     * add_instanced_object: cria um sub-objeto instanciado e o adiciona a um array.
     *
     * E o que faltava para popular `USBItemDefinition::Fragments`, que e um TArray de
     * TObjectPtr marcado como Instanced — cada elemento precisa ser um UObject novo com o
     * proprio asset como outer. Um setter de propriedade generico nao da conta disso.
     */
    TSharedPtr<FJsonObject> HandleAddInstancedObject(const TSharedPtr<FJsonObject>& Params);

    /** get_data_asset_properties: le o asset de volta, para verificacao por estado. */
    TSharedPtr<FJsonObject> HandleGetDataAssetProperties(const TSharedPtr<FJsonObject>& Params);
};
