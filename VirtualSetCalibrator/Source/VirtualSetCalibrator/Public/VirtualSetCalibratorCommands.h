// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "VirtualSetCalibratorStyle.h"

class FVirtualSetCalibratorCommands : public TCommands<FVirtualSetCalibratorCommands>
{
public:

	FVirtualSetCalibratorCommands()
		: TCommands<FVirtualSetCalibratorCommands>(TEXT("VirtualSetCalibrator"), NSLOCTEXT("Contexts", "VirtualSetCalibrator", "VirtualSetCalibrator Plugin"), NAME_None, FVirtualSetCalibratorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
