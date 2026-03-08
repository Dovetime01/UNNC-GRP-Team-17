#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;
class STextBlock;

struct FVPWorkflowIntelligenceGenerateResult
{
	bool bSucceeded = false;
	FString StatusMessage;
	FString ResponseText;
	double InitializeDurationSeconds = 0.0;
	double GenerateDurationSeconds = 0.0;
	FString ModelPath;
	FString Prompt;
};

class SVPWorkflowIntelligencePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SVPWorkflowIntelligencePanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	FReply HandleBrowseModelClicked();
	FReply HandleGenerateClicked();
	FReply HandleClearClicked();

	bool CanGenerate() const;
	FString GetModelPath() const;
	FString GetPrompt() const;
	FString ResolveInitialModelPath() const;
	void SetStatusMessage(const FString& InMessage, const FLinearColor& InColor);
	void HandleGenerationCompleted(const FVPWorkflowIntelligenceGenerateResult& Result);

private:
	TSharedPtr<SEditableTextBox> ModelPathTextBox;
	TSharedPtr<SMultiLineEditableTextBox> PromptTextBox;
	TSharedPtr<SMultiLineEditableTextBox> ResponseTextBox;
	TSharedPtr<STextBlock> StatusTextBlock;
	FSlateColor StatusColor;
	bool bIsGenerating = false;
};