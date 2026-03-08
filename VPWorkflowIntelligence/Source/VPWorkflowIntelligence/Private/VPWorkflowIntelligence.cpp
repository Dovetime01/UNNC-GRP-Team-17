// Copyright Epic Games, Inc. All Rights Reserved.

#include "VPWorkflowIntelligence.h"
#include "VPWorkflowIntelligenceStyle.h"
#include "VPWorkflowIntelligenceCommands.h"
#include "LlamaService.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "SlateBasics.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "Editor/Blutility/Classes/EditorUtilityWidget.h"
#include "Editor/UMGEditor/Public/WidgetBlueprint.h"
#include "Editor/LevelEditor/Public/LevelEditor.h"
#include "Editor/Blutility/Public/IBlutilityModule.h"
#include "Editor/Blutility/Classes/EditorUtilityWidgetBlueprint.h"
#include <Editor/Blutility/Public/EditorUtilitySubsystem.h>

static const FName VPWorkflowIntelligenceTabName("VPWorkflowIntelligence");

#define LOCTEXT_NAMESPACE "FVPWorkflowIntelligenceModule"

void FVPWorkflowIntelligenceModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FVPWorkflowIntelligenceStyle::Initialize();
	FVPWorkflowIntelligenceStyle::ReloadTextures();

	FVPWorkflowIntelligenceCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVPWorkflowIntelligenceCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVPWorkflowIntelligenceModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVPWorkflowIntelligenceModule::RegisterMenus));
}

void FVPWorkflowIntelligenceModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FVPWorkflowIntelligenceStyle::Shutdown();

	FVPWorkflowIntelligenceCommands::Unregister();
}

void FVPWorkflowIntelligenceModule::PluginButtonClicked()
{
	const FString TestModelPath = TEXT("C:/Users/dylon/Documents/Unreal Projects/Pico/Plugins/VPWorkflowIntelligence/Source/ThirdParty/llama.cpp/models/Qwen3.5-4B-Q4_K_M (1).gguf");
	const double InitializeStartTime = FPlatformTime::Seconds();

	FLlamaService LlamaService;
	const bool bInitializeSucceeded = LlamaService.Initialize(TestModelPath);
	const double InitializeDurationSeconds = FPlatformTime::Seconds() - InitializeStartTime;

	if (bInitializeSucceeded)
	{
		const FString SuccessMessage = FString::Printf(
			TEXT("FLlamaService::Initialize succeeded in %.2f s"),
			InitializeDurationSeconds);

		UE_LOG(LogTemp, Log, TEXT("%s. Model: %s"), *SuccessMessage, *LlamaService.GetModelPath());
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Green, SuccessMessage);

		const FString TestPrompt = TEXT("请用一句简短的话介绍你自己。请用中文回答。");
		const double GenerateStartTime = FPlatformTime::Seconds();
		const FString GeneratedText = LlamaService.Generate(TestPrompt);
		const double GenerateDurationSeconds = FPlatformTime::Seconds() - GenerateStartTime;

		UE_LOG(LogTemp, Log, TEXT("FLlamaService::Generate finished in %.2f s. Prompt: %s"), GenerateDurationSeconds, *TestPrompt);
		UE_LOG(LogTemp, Log, TEXT("FLlamaService::Generate response: %s"), *GeneratedText);
		GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Cyan, FString::Printf(TEXT("Generate done in %.2f s"), GenerateDurationSeconds));

		LlamaService.Shutdown();
	}
	else
	{
		const FString FailureMessage = FString::Printf(
			TEXT("FLlamaService::Initialize failed: %s"),
			*LlamaService.GetLastError());

		UE_LOG(LogTemp, Error, TEXT("%s"), *FailureMessage);
		GEngine->AddOnScreenDebugMessage(-1, 12.f, FColor::Red, FailureMessage);
		return;
	}

	FString vpWidgetPath = "/VPWorkflowIntelligence/UI/hh.hh";
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Button Clicked: %s"), *vpWidgetPath));

	// ��ȡ Editor Utility Subsystem
	if (!GEditor)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GEditor is null!"));
		return;
	}

	UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
	if (!EditorUtilitySubsystem)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("EditorUtilitySubsystem not found!"));
		return;
	}

	// ���� Editor Utility Widget Blueprint
	UEditorUtilityWidgetBlueprint* WidgetBlueprint = Cast<UEditorUtilityWidgetBlueprint>(StaticLoadObject(UEditorUtilityWidgetBlueprint::StaticClass(), nullptr, *vpWidgetPath));
	if (!WidgetBlueprint)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Widget Blueprint not found at path: %s"), *vpWidgetPath));
		return;
	}

	// ��������ʾ Editor Utility Widget
	EditorUtilitySubsystem->SpawnAndRegisterTab(WidgetBlueprint);
}

void FVPWorkflowIntelligenceModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{

			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVPWorkflowIntelligenceCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVPWorkflowIntelligenceCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVPWorkflowIntelligenceModule, VPWorkflowIntelligence)