// Copyright Epic Games, Inc. All Rights Reserved.

#include "VirtualSetCalibratorCommands.h"

#define LOCTEXT_NAMESPACE "FVirtualSetCalibratorModule"

void FVirtualSetCalibratorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Virtual Set Calibrator", "Open the Virtual Set Calibrator panel", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
