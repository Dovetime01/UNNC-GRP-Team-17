// Copyright Epic Games, Inc. All Rights Reserved.

#include "VPWorkflowIntelligenceStyle.h"
#include "VPWorkflowIntelligence.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FVPWorkflowIntelligenceStyle::StyleInstance = nullptr;

void FVPWorkflowIntelligenceStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FVPWorkflowIntelligenceStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FVPWorkflowIntelligenceStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("VPWorkflowIntelligenceStyle"));
	return StyleSetName;
}


const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FVPWorkflowIntelligenceStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("VPWorkflowIntelligenceStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("VPWorkflowIntelligence")->GetBaseDir() / TEXT("Resources"));

	Style->Set("VPWorkflowIntelligence.PluginAction", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));
	return Style;
}

void FVPWorkflowIntelligenceStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FVPWorkflowIntelligenceStyle::Get()
{
	return *StyleInstance;
}
