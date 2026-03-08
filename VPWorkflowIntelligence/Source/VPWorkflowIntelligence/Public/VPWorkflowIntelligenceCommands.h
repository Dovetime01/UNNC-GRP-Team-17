// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "VPWorkflowIntelligenceStyle.h"

class FVPWorkflowIntelligenceCommands : public TCommands<FVPWorkflowIntelligenceCommands>
{
public:

	FVPWorkflowIntelligenceCommands()
		: TCommands<FVPWorkflowIntelligenceCommands>(TEXT("VPWorkflowIntelligence"), NSLOCTEXT("Contexts", "VPWorkflowIntelligence", "VPWorkflowIntelligence Plugin"), NAME_None, FVPWorkflowIntelligenceStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
