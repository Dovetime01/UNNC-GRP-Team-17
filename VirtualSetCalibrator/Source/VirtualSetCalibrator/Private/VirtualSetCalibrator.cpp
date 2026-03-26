// Copyright Epic Games, Inc. All Rights Reserved.

#include "VirtualSetCalibrator.h"
#include "VirtualSetCalibratorStyle.h"
#include "VirtualSetCalibratorCommands.h"
#include "Panel/VSCalibratorPanel.h"
#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

static const FName VirtualSetCalibratorTabName("VirtualSetCalibrator");

#define LOCTEXT_NAMESPACE "FVirtualSetCalibratorModule"

void FVirtualSetCalibratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		VirtualSetCalibratorTabName,
		FOnSpawnTab::CreateRaw(this, &FVirtualSetCalibratorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("VirtualSetCalibratorTabTitle", "Virtual Set Calibrator"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	FVirtualSetCalibratorStyle::Initialize();
	FVirtualSetCalibratorStyle::ReloadTextures();

	FVirtualSetCalibratorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FVirtualSetCalibratorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FVirtualSetCalibratorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FVirtualSetCalibratorModule::RegisterMenus));
}

void FVirtualSetCalibratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(VirtualSetCalibratorTabName);

	FVirtualSetCalibratorStyle::Shutdown();

	FVirtualSetCalibratorCommands::Unregister();
}

void FVirtualSetCalibratorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(VirtualSetCalibratorTabName);
}

TSharedRef<SDockTab> FVirtualSetCalibratorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVSCalibratorPanel)
		];
}

void FVirtualSetCalibratorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FVirtualSetCalibratorCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FVirtualSetCalibratorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FVirtualSetCalibratorModule, VirtualSetCalibrator)
