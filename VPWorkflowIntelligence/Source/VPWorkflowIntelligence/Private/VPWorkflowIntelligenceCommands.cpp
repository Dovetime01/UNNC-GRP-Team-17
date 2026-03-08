// Copyright Epic Games, Inc. All Rights Reserved.

#include "VPWorkflowIntelligenceCommands.h"

#define LOCTEXT_NAMESPACE "FVPWorkflowIntelligenceModule"

void FVPWorkflowIntelligenceCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "VPWorkflowIntelligence", "Execute VPWorkflowIntelligence action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
