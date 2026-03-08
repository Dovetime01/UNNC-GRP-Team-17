// Copyright Epic Games, Inc. All Rights Reserved.

#include "VPWorkflowIntelligenceCommands.h"

#define LOCTEXT_NAMESPACE "FVPWorkflowIntelligenceModule"

void FVPWorkflowIntelligenceCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "VP Workflow Intelligence", "Open the VP Workflow Intelligence panel", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
