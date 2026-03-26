// Copyright Epic Games, Inc. All Rights Reserved.

#include "VirtualSetCalibratorStyle.h"
#include "VirtualSetCalibrator.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FVirtualSetCalibratorStyle::StyleInstance = nullptr;

void FVirtualSetCalibratorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVirtualSetCalibratorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FVirtualSetCalibratorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VirtualSetCalibratorStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FVirtualSetCalibratorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("VirtualSetCalibratorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("VirtualSetCalibrator")->GetBaseDir() / TEXT("Resources"));

	Style->Set("VirtualSetCalibrator.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FVirtualSetCalibratorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FVirtualSetCalibratorStyle::Get()
{
	return *StyleInstance;
}
