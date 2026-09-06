#include "Commands/UnrealMCPUMGCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "K2Node_ComponentBoundEvent.h"
// We'll create widgets using regular Factory classes
#include "Factories/Factory.h"
// Remove problematic includes that don't exist in UE 5.5
// #include "UMGEditorSubsystem.h"
// #include "WidgetBlueprintFactory.h"
#include "WidgetBlueprintEditor.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "JsonObjectConverter.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Components/Button.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CallFunction.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "K2Node_Event.h"

FUnrealMCPUMGCommands::FUnrealMCPUMGCommands()
{
}

namespace
{
    /**
     * Desambigua pai e filho entre as duas convencoes de nome que este plugin recebe.
     *
     * Esquema Python:  { "widget_name": <blueprint pai>, "text_block_name": <filho> }
     * Esquema C++:     { "blueprint_name": <blueprint pai>, "widget_name": <filho> }
     *
     * `widget_name` significa coisas opostas nos dois. A regra que resolve sem ambiguidade: se
     * veio um nome de filho especifico, entao `widget_name` e o pai; caso contrario `widget_name`
     * e o filho e o pai veio em `blueprint_name`.
     */
    bool ResolveWidgetTarget(const TSharedPtr<FJsonObject>& Params, FString& OutParent, FString& OutChild)
    {
        const TArray<FString> ChildSpecific = {
            TEXT("text_block_name"), TEXT("button_name"),
            TEXT("widget_component_name"), TEXT("component_name")
        };

        const bool bHasSpecificChild = FUnrealMCPCommonUtils::GetStringParam(Params, ChildSpecific, OutChild);

        if (bHasSpecificChild)
        {
            return FUnrealMCPCommonUtils::GetStringParam(
                Params, {TEXT("blueprint_name"), TEXT("widget_name"), TEXT("name")}, OutParent);
        }

        const bool bHasParent = FUnrealMCPCommonUtils::GetStringParam(
            Params, {TEXT("blueprint_name"), TEXT("name")}, OutParent);
        const bool bHasChild = FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("widget_name")}, OutChild);

        return bHasParent && bHasChild;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCommand(const FString& CommandName, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandName == TEXT("create_umg_widget_blueprint"))
	{
		return HandleCreateUMGWidgetBlueprint(Params);
	}
	else if (CommandName == TEXT("add_text_block_to_widget"))
	{
		return HandleAddTextBlockToWidget(Params);
	}
	else if (CommandName == TEXT("add_widget_to_viewport"))
	{
		return HandleAddWidgetToViewport(Params);
	}
	else if (CommandName == TEXT("add_button_to_widget"))
	{
		return HandleAddButtonToWidget(Params);
	}
	else if (CommandName == TEXT("bind_widget_event"))
	{
		return HandleBindWidgetEvent(Params);
	}
	else if (CommandName == TEXT("set_text_block_binding"))
	{
		return HandleSetTextBlockBinding(Params);
	}

	return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown UMG command: %s"), *CommandName));
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleCreateUMGWidgetBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("name"), TEXT("widget_name"), TEXT("blueprint_name")}, BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' (ou 'widget_name') parameter"));
	}

	// O parametro `path` existia no schema e era ignorado: o destino era sempre /Game/Widgets.
	FString PackagePath = TEXT("/Game/Widgets/");
	Params->TryGetStringField(TEXT("path"), PackagePath);
	if (!PackagePath.EndsWith(TEXT("/")))
	{
		PackagePath += TEXT("/");
	}
	FString AssetName = BlueprintName;
	FString FullPath = BlueprintName.StartsWith(TEXT("/")) ? BlueprintName : PackagePath + AssetName;

	// Check if asset already exists
	if (UEditorAssetLibrary::DoesAssetExist(FullPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' already exists"), *BlueprintName));
	}

	// Create package
	UPackage* Package = CreatePackage(*FullPath);
	if (!Package)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create package"));
	}

	// Widget Blueprint exige as classes proprias de UMG. A versao anterior pedia
	// UBlueprint/UBlueprintGeneratedClass, entao o objeto criado nunca era um UWidgetBlueprint e
	// o Cast abaixo falhava sempre — este comando nunca funcionou, nem com os parametros certos.
	UBlueprint* NewBlueprint = FKismetEditorUtilities::CreateBlueprint(
		UUserWidget::StaticClass(),
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UWidgetBlueprint::StaticClass(),
		UWidgetBlueprintGeneratedClass::StaticClass(),
		FName("CreateUMGWidget")
	);

	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(NewBlueprint);
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
			TEXT("Failed to create Widget Blueprint at '%s' (obtido: %s)"),
			*FullPath, NewBlueprint ? *NewBlueprint->GetClass()->GetName() : TEXT("nullptr")));
	}

	// Add a default Canvas Panel if one doesn't exist
	if (!WidgetBlueprint->WidgetTree->RootWidget)
	{
		UCanvasPanel* RootCanvas = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
		WidgetBlueprint->WidgetTree->RootWidget = RootCanvas;
		if (RootCanvas && !WidgetBlueprint->WidgetVariableNameToGuidMap.Contains(RootCanvas->GetFName()))
		{
			WidgetBlueprint->OnVariableAdded(RootCanvas->GetFName());
		}
	}

	// Mark the package dirty and notify asset registry
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(WidgetBlueprint);

	// Compile the blueprint
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	const bool bSaved = FUnrealMCPCommonUtils::SaveAssetToDisk(WidgetBlueprint);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("name"), BlueprintName);
	ResultObj->SetStringField(TEXT("path"), FullPath);
	ResultObj->SetBoolField(TEXT("saved_to_disk"), bSaved);
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddTextBlockToWidget(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	FString WidgetName;
	if (!ResolveWidgetTarget(Params, BlueprintName, WidgetName) || BlueprintName.IsEmpty() || WidgetName.IsEmpty())
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(
			TEXT("Missing widget target. Envie 'widget_name' + 'text_block_name'/'button_name', ou 'blueprint_name' + 'widget_name'."));
	}

	// Find the Widget Blueprint
	FString FullPath = FUnrealMCPCommonUtils::ResolveAssetPath(BlueprintName, TEXT("/Game/Widgets/"));
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(FullPath));
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional parameters
	FString InitialText = TEXT("New Text Block");
	Params->TryGetStringField(TEXT("text"), InitialText);

	FVector2D Position(0.0f, 0.0f);
	if (Params->HasField(TEXT("position")))
	{
		const TArray<TSharedPtr<FJsonValue>>* PosArray;
		if (Params->TryGetArrayField(TEXT("position"), PosArray) && PosArray->Num() >= 2)
		{
			Position.X = (*PosArray)[0]->AsNumber();
			Position.Y = (*PosArray)[1]->AsNumber();
		}
	}

	// Create Text Block widget
	UTextBlock* TextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *WidgetName);
	if (!TextBlock)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Text Block widget"));
	}

	// Registrar a variavel no mapa de GUIDs do Widget Blueprint. A UE 5 exige isso de todo widget
	// nomeado da arvore: WidgetBlueprintCompiler.cpp:760 dispara ensure quando encontra um widget
	// ausente do mapa, e o compilador so preenche sozinho quando o mapa esta inteiramente vazio.
	// Sem esta chamada, adicionar um widget por codigo estoura ensure na primeira compilacao.
	// Sem bIsVariable a classe compilada nao ganha FObjectProperty para este widget, e sem essa
	// propriedade nao ha como criar evento vinculado (bind_widget_event).
	TextBlock->bIsVariable = true;
	if (!WidgetBlueprint->WidgetVariableNameToGuidMap.Contains(TextBlock->GetFName()))
	{
		WidgetBlueprint->OnVariableAdded(TextBlock->GetFName());
	}

	// Set initial text
	TextBlock->SetText(FText::FromString(InitialText));

	// Add to canvas panel
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root Canvas Panel not found"));
	}

	UCanvasPanelSlot* PanelSlot = RootCanvas->AddChildToCanvas(TextBlock);
	PanelSlot->SetPosition(Position);

	// Mark the package dirty and compile
	WidgetBlueprint->MarkPackageDirty();
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);

	// Create success response
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("widget_name"), WidgetName);
	ResultObj->SetStringField(TEXT("text"), InitialText);
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddWidgetToViewport(const TSharedPtr<FJsonObject>& Params)
{
	// Get required parameters
	FString BlueprintName;
	if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("blueprint_name"), TEXT("widget_name"), TEXT("name")}, BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'widget_name' (ou 'blueprint_name') parameter"));
	}

	// Find the Widget Blueprint
	FString FullPath = FUnrealMCPCommonUtils::ResolveAssetPath(BlueprintName, TEXT("/Game/Widgets/"));
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(FullPath));
	if (!WidgetBlueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Widget Blueprint '%s' not found"), *BlueprintName));
	}

	// Get optional Z-order parameter
	int32 ZOrder = 0;
	Params->TryGetNumberField(TEXT("z_order"), ZOrder);

	// Create widget instance
	UClass* WidgetClass = WidgetBlueprint->GeneratedClass;
	if (!WidgetClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get widget class"));
	}

	// Note: This creates the widget but doesn't add it to viewport
	// The actual addition to viewport should be done through Blueprint nodes
	// as it requires a game context

	// Create success response with instructions
	TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
	ResultObj->SetStringField(TEXT("blueprint_name"), BlueprintName);
	ResultObj->SetStringField(TEXT("class_path"), WidgetClass->GetPathName());
	ResultObj->SetNumberField(TEXT("z_order"), ZOrder);
	ResultObj->SetStringField(TEXT("note"), TEXT("Widget class ready. Use CreateWidget and AddToViewport nodes in Blueprint to display in game."));
	return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleAddButtonToWidget(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	FString WidgetName;
	if (!ResolveWidgetTarget(Params, BlueprintName, WidgetName) || BlueprintName.IsEmpty() || WidgetName.IsEmpty())
	{
		// Antes isto devolvia {"status":"success","result":{"error":"..."}} — falha vestida de
		// sucesso, que um chamador que le so o `status` interpreta como tendo funcionado.
		return FUnrealMCPCommonUtils::CreateErrorResponse(
			TEXT("Missing widget target. Envie 'widget_name' + 'text_block_name'/'button_name', ou 'blueprint_name' + 'widget_name'."));
	}

	FString ButtonText;
	if (!Params->TryGetStringField(TEXT("text"), ButtonText))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing text parameter"));
		return Response;
	}

	// Load the Widget Blueprint
	const FString ResolvedPackage = FUnrealMCPCommonUtils::ResolveAssetPath(BlueprintName, TEXT("/Game/Widgets/"));
	FString ShortName; ResolvedPackage.Split(TEXT("/"), nullptr, &ShortName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString BlueprintPath = FString::Printf(TEXT("%s.%s"), *ResolvedPackage, *ShortName);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
		return Response;
	}

	// Construir pela WidgetTree, nao com NewObject sobre o CDO da classe gerada. Widget criado
	// fora da arvore nao pertence ao Blueprint: nao aparece no Designer, nao vira variavel e some
	// na recompilacao.
	UButton* Button = WidgetBlueprint->WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), *WidgetName);
	if (!Button)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Button widget"));
	}

	// Sem bIsVariable a classe compilada nao ganha FObjectProperty para este widget, e sem essa
	// propriedade nao ha como criar evento vinculado (bind_widget_event).
	Button->bIsVariable = true;
	if (!WidgetBlueprint->WidgetVariableNameToGuidMap.Contains(Button->GetFName()))
	{
		WidgetBlueprint->OnVariableAdded(Button->GetFName());
	}

	// Set button text
	UTextBlock* ButtonTextBlock = WidgetBlueprint->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *(WidgetName + TEXT("_Text")));
	if (ButtonTextBlock)
	{
		ButtonTextBlock->SetText(FText::FromString(ButtonText));
		Button->AddChild(ButtonTextBlock);
		if (!WidgetBlueprint->WidgetVariableNameToGuidMap.Contains(ButtonTextBlock->GetFName()))
		{
			WidgetBlueprint->OnVariableAdded(ButtonTextBlock->GetFName());
		}
	}

	// Get canvas panel and add button
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(WidgetBlueprint->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Root widget is not a Canvas Panel"));
	}

	// Add to canvas and set position
	UCanvasPanelSlot* ButtonSlot = RootCanvas->AddChildToCanvas(Button);
	if (ButtonSlot)
	{
		const TArray<TSharedPtr<FJsonValue>>* Position;
		if (Params->TryGetArrayField(TEXT("position"), Position) && Position->Num() >= 2)
		{
			FVector2D Pos(
				(*Position)[0]->AsNumber(),
				(*Position)[1]->AsNumber()
			);
			ButtonSlot->SetPosition(Pos);
		}
	}

	// Save the Widget Blueprint
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(BlueprintPath, false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("widget_name"), WidgetName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleBindWidgetEvent(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	FString WidgetName;
	if (!ResolveWidgetTarget(Params, BlueprintName, WidgetName) || BlueprintName.IsEmpty() || WidgetName.IsEmpty())
	{
		// Antes isto devolvia {"status":"success","result":{"error":"..."}} — falha vestida de
		// sucesso, que um chamador que le so o `status` interpreta como tendo funcionado.
		return FUnrealMCPCommonUtils::CreateErrorResponse(
			TEXT("Missing widget target. Envie 'widget_name' + 'text_block_name'/'button_name', ou 'blueprint_name' + 'widget_name'."));
	}

	FString EventName;
	if (!Params->TryGetStringField(TEXT("event_name"), EventName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing event_name parameter"));
		return Response;
	}

	// Load the Widget Blueprint
	const FString ResolvedPackage = FUnrealMCPCommonUtils::ResolveAssetPath(BlueprintName, TEXT("/Game/Widgets/"));
	FString ShortName; ResolvedPackage.Split(TEXT("/"), nullptr, &ShortName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString BlueprintPath = FString::Printf(TEXT("%s.%s"), *ResolvedPackage, *ShortName);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
		return Response;
	}

	// Create the event graph if it doesn't exist
	UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(WidgetBlueprint);
	if (!EventGraph)
	{
		Response->SetStringField(TEXT("error"), TEXT("Failed to find or create event graph"));
		return Response;
	}

	// Find the widget in the blueprint
	UWidget* Widget = WidgetBlueprint->WidgetTree->FindWidget(*WidgetName);
	if (!Widget)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to find widget: %s"), *WidgetName));
		return Response;
	}

	// Create the event node (e.g., OnClicked for buttons)
	UK2Node_Event* EventNode = nullptr;
	
	// Find existing nodes first
	TArray<UK2Node_Event*> AllEventNodes;
	FBlueprintEditorUtils::GetAllNodesOfClass<UK2Node_Event>(WidgetBlueprint, AllEventNodes);
	
	for (UK2Node_Event* Node : AllEventNodes)
	{
		if (Node->CustomFunctionName == FName(*EventName) && Node->EventReference.GetMemberParentClass() == Widget->GetClass())
		{
			EventNode = Node;
			break;
		}
	}

	// If no existing node, create a new one
	if (!EventNode)
	{
		// Calculate position - place it below existing nodes
		float MaxHeight = 0.0f;
		for (UEdGraphNode* Node : EventGraph->Nodes)
		{
			MaxHeight = FMath::Max(MaxHeight, Node->NodePosY);
		}
		
		const FVector2D NodePos(200, MaxHeight + 200);

		// Evento vinculado exige a FObjectProperty do widget na classe compilada. Antes era
		// passado nullptr, e sem a propriedade nao ha o que vincular — o no nunca era criado.
		FObjectProperty* WidgetProperty = FindFProperty<FObjectProperty>(
			WidgetBlueprint->SkeletonGeneratedClass ? WidgetBlueprint->SkeletonGeneratedClass : WidgetBlueprint->GeneratedClass,
			*WidgetName);

		if (!WidgetProperty)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
				TEXT("O widget '%s' nao e uma variavel da classe compilada. Recompile o Widget Blueprint apos adiciona-lo."),
				*WidgetName));
		}

		FKismetEditorUtilities::CreateNewBoundEventForComponent(
			Widget,
			FName(*EventName),
			WidgetBlueprint,
			WidgetProperty,
			/*bShouldJumpToNode=*/false
		);

		// Evento vinculado nao usa CustomFunctionName igual ao nome do evento: o nome gerado tem
		// a forma BndEvt__<widget>_<...>. Procurar pela API propria da engine.
		if (const UK2Node_ComponentBoundEvent* Bound =
				FKismetEditorUtilities::FindBoundEventForComponent(WidgetBlueprint, FName(*EventName), FName(*WidgetName)))
		{
			EventNode = const_cast<UK2Node_ComponentBoundEvent*>(Bound);
			EventNode->NodePosX = NodePos.X;
			EventNode->NodePosY = NodePos.Y;
		}
	}

	if (!EventNode)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(
			TEXT("Nao foi possivel criar o evento '%s' para o widget '%s'."), *EventName, *WidgetName));
	}

	// Save the Widget Blueprint
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(BlueprintPath, false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("event_name"), EventName);
	return Response;
}

TSharedPtr<FJsonObject> FUnrealMCPUMGCommands::HandleSetTextBlockBinding(const TSharedPtr<FJsonObject>& Params)
{
	TSharedPtr<FJsonObject> Response = MakeShared<FJsonObject>();

	// Get required parameters
	FString BlueprintName;
	FString WidgetName;
	if (!ResolveWidgetTarget(Params, BlueprintName, WidgetName) || BlueprintName.IsEmpty() || WidgetName.IsEmpty())
	{
		// Antes isto devolvia {"status":"success","result":{"error":"..."}} — falha vestida de
		// sucesso, que um chamador que le so o `status` interpreta como tendo funcionado.
		return FUnrealMCPCommonUtils::CreateErrorResponse(
			TEXT("Missing widget target. Envie 'widget_name' + 'text_block_name'/'button_name', ou 'blueprint_name' + 'widget_name'."));
	}

	FString BindingName;
	if (!FUnrealMCPCommonUtils::GetStringParam(Params, {TEXT("binding_name"), TEXT("binding_property")}, BindingName))
	{
		Response->SetStringField(TEXT("error"), TEXT("Missing binding_name parameter"));
		return Response;
	}

	// Load the Widget Blueprint
	const FString ResolvedPackage = FUnrealMCPCommonUtils::ResolveAssetPath(BlueprintName, TEXT("/Game/Widgets/"));
	FString ShortName; ResolvedPackage.Split(TEXT("/"), nullptr, &ShortName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	const FString BlueprintPath = FString::Printf(TEXT("%s.%s"), *ResolvedPackage, *ShortName);
	UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(UEditorAssetLibrary::LoadAsset(BlueprintPath));
	if (!WidgetBlueprint)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to load Widget Blueprint: %s"), *BlueprintPath));
		return Response;
	}

	// Create a variable for binding if it doesn't exist
	FBlueprintEditorUtils::AddMemberVariable(
		WidgetBlueprint,
		FName(*BindingName),
		FEdGraphPinType(UEdGraphSchema_K2::PC_Text, NAME_None, nullptr, EPinContainerType::None, false, FEdGraphTerminalType())
	);

	// Find the TextBlock widget
	UTextBlock* TextBlock = Cast<UTextBlock>(WidgetBlueprint->WidgetTree->FindWidget(FName(*WidgetName)));
	if (!TextBlock)
	{
		Response->SetStringField(TEXT("error"), FString::Printf(TEXT("Failed to find TextBlock widget: %s"), *WidgetName));
		return Response;
	}

	// Create binding function
	const FString FunctionName = FString::Printf(TEXT("Get%s"), *BindingName);
	UEdGraph* FuncGraph = FBlueprintEditorUtils::CreateNewGraph(
		WidgetBlueprint,
		FName(*FunctionName),
		UEdGraph::StaticClass(),
		UEdGraphSchema_K2::StaticClass()
	);

	if (FuncGraph)
	{
		// Add the function to the blueprint with proper template parameter
		// Template requires null for last parameter when not using a signature-source
		FBlueprintEditorUtils::AddFunctionGraph<UClass>(WidgetBlueprint, FuncGraph, false, nullptr);

		// Create entry node
		UK2Node_FunctionEntry* EntryNode = nullptr;
		
		// Create entry node - use the API that exists in UE 5.5
		EntryNode = NewObject<UK2Node_FunctionEntry>(FuncGraph);
		FuncGraph->AddNode(EntryNode, false, false);
		EntryNode->NodePosX = 0;
		EntryNode->NodePosY = 0;
		EntryNode->FunctionReference.SetExternalMember(FName(*FunctionName), WidgetBlueprint->GeneratedClass);
		EntryNode->AllocateDefaultPins();

		// Create get variable node
		UK2Node_VariableGet* GetVarNode = NewObject<UK2Node_VariableGet>(FuncGraph);
		GetVarNode->VariableReference.SetSelfMember(FName(*BindingName));
		FuncGraph->AddNode(GetVarNode, false, false);
		GetVarNode->NodePosX = 200;
		GetVarNode->NodePosY = 0;
		GetVarNode->AllocateDefaultPins();

		// Connect nodes
		UEdGraphPin* EntryThenPin = EntryNode->FindPin(UEdGraphSchema_K2::PN_Then);
		UEdGraphPin* GetVarOutPin = GetVarNode->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		if (EntryThenPin && GetVarOutPin)
		{
			EntryThenPin->MakeLinkTo(GetVarOutPin);
		}
	}

	// Save the Widget Blueprint
	FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
	UEditorAssetLibrary::SaveAsset(BlueprintPath, false);

	Response->SetBoolField(TEXT("success"), true);
	Response->SetStringField(TEXT("binding_name"), BindingName);
	return Response;
} 