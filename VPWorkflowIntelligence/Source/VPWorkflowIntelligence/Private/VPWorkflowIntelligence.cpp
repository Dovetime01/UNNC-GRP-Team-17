// Copyright Epic Games, Inc. All Rights Reserved.

#include "VPWorkflowIntelligence.h"
#include "SVPWorkflowIntelligencePanel.h"
#include "VPWorkflowIntelligenceStyle.h"
#include "VPWorkflowIntelligenceCommands.h"
#include "ToolMenus.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "Modules/ModuleManager.h"

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

	EnsureTabSpawnerRegistered();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVPWorkflowIntelligenceModule::RegisterMenus));
}

void FVPWorkflowIntelligenceModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	if (FGlobalTabmanager::Get()->HasTabSpawner(VPWorkflowIntelligenceTabName))
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VPWorkflowIntelligenceTabName);
	}

	FVPWorkflowIntelligenceStyle::Shutdown();

	FVPWorkflowIntelligenceCommands::Unregister();
}

void FVPWorkflowIntelligenceModule::PluginButtonClicked()
{
	EnsureTabSpawnerRegistered();
	FGlobalTabmanager::Get()->TryInvokeTab(VPWorkflowIntelligenceTabName);
}

void FVPWorkflowIntelligenceModule::EnsureTabSpawnerRegistered()
{
	if (FGlobalTabmanager::Get()->HasTabSpawner(VPWorkflowIntelligenceTabName))
	{
		return;
	}

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VPWorkflowIntelligenceTabName,
		FOnSpawnTab::CreateRaw(this, &FVPWorkflowIntelligenceModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("VPWorkflowIntelligenceTabTitle", "VP Workflow Intelligence"))
		.SetMenuType(ETabSpawnerMenuType::Hidden)
		.SetIcon(FSlateIcon(FVPWorkflowIntelligenceStyle::GetStyleSetName(), "VPWorkflowIntelligence.PluginAction"));
}

TSharedRef<SDockTab> FVPWorkflowIntelligenceModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(LOCTEXT("VPWorkflowIntelligencePanelLabel", "VP Workflow Intelligence"))
		[
			SNew(SVPWorkflowIntelligencePanel)
		];
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