#include "SVPWorkflowIntelligencePanel.h"

#include "Async/Async.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "LlamaService.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/AppStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
	const FString DefaultPrompt = TEXT("请用一句简短的话介绍你自己。请用中文回答。");
}

void SVPWorkflowIntelligencePanel::Construct(const FArguments& InArgs)
{
	StatusColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(16.0f)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("VP Workflow Intelligence")))
					.Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 12.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("当前面板提供本地 GGUF 模型选择、Prompt 输入、异步单次生成与结果回看。当前实现为安全的一次性请求模式，每次生成都会独立完成加载、推理与释放。")))
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor(0.75f, 0.75f, 0.75f, 1.0f)))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 14.0f)
				[
					SNew(SSeparator)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 0.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("模型路径")))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SAssignNew(ModelPathTextBox, SEditableTextBox)
						.Text(FText::FromString(ResolveInitialModelPath()))
						.HintText(FText::FromString(TEXT("选择一个 .gguf 模型文件")))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(FText::FromString(TEXT("浏览...")))
						.OnClicked(this, &SVPWorkflowIntelligencePanel::HandleBrowseModelClicked)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 16.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Prompt")))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SBox)
					.MinDesiredHeight(150.0f)
					[
						SAssignNew(PromptTextBox, SMultiLineEditableTextBox)
						.Text(FText::FromString(DefaultPrompt))
						.HintText(FText::FromString(TEXT("输入希望发送给本地模型的内容")))
						.AutoWrapText(true)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 16.0f, 0.0f, 12.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.IsEnabled(this, &SVPWorkflowIntelligencePanel::CanGenerate)
						.Text(FText::FromString(TEXT("生成回复")))
						.OnClicked(this, &SVPWorkflowIntelligencePanel::HandleGenerateClicked)
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(8.0f, 0.0f, 0.0f, 0.0f)
					[
						SNew(SButton)
						.IsEnabled_Lambda([this]() { return !bIsGenerating; })
						.Text(FText::FromString(TEXT("清空内容")))
						.OnClicked(this, &SVPWorkflowIntelligencePanel::HandleClearClicked)
					]
					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(12.0f, 0.0f, 0.0f, 0.0f)
					.VAlign(VAlign_Center)
					[
						SAssignNew(StatusTextBlock, STextBlock)
						.Text(FText::FromString(TEXT("就绪。")))
						.AutoWrapText(true)
						.ColorAndOpacity(StatusColor)
					]
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 16.0f, 0.0f, 8.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("模型输出")))
					.Font(FAppStyle::GetFontStyle("PropertyWindow.BoldFont"))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SBox)
					.MinDesiredHeight(220.0f)
					[
						SAssignNew(ResponseTextBox, SMultiLineEditableTextBox)
						.IsReadOnly(true)
						.HintText(FText::FromString(TEXT("生成结果会显示在这里")))
						.AutoWrapText(true)
					]
				]
			]
		]
	];
}

FReply SVPWorkflowIntelligencePanel::HandleBrowseModelClicked()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		SetStatusMessage(TEXT("无法打开文件选择器：DesktopPlatform 不可用。"), FLinearColor(0.9f, 0.3f, 0.3f, 1.0f));
		return FReply::Handled();
	}

	const FString CurrentModelPath = GetModelPath();
	const FString DefaultDirectory = CurrentModelPath.IsEmpty() ? FPaths::ProjectDir() : FPaths::GetPath(CurrentModelPath);
	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
	TArray<FString> SelectedFiles;

	const bool bFileSelected = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("选择 GGUF 模型"),
		DefaultDirectory,
		TEXT(""),
		TEXT("GGUF Model (*.gguf)|*.gguf"),
		EFileDialogFlags::None,
		SelectedFiles);

	if (bFileSelected && SelectedFiles.Num() > 0 && ModelPathTextBox.IsValid())
	{
		ModelPathTextBox->SetText(FText::FromString(SelectedFiles[0]));
		SetStatusMessage(TEXT("已更新模型路径。"), FLinearColor(0.4f, 0.8f, 1.0f, 1.0f));
	}

	return FReply::Handled();
}

FReply SVPWorkflowIntelligencePanel::HandleGenerateClicked()
{
	if (!CanGenerate())
	{
		return FReply::Handled();
	}

	const FString ModelPath = GetModelPath();
	const FString Prompt = GetPrompt();

	if (ModelPath.IsEmpty())
	{
		SetStatusMessage(TEXT("请先指定有效的 GGUF 模型路径。"), FLinearColor(0.9f, 0.3f, 0.3f, 1.0f));
		return FReply::Handled();
	}

	if (Prompt.IsEmpty())
	{
		SetStatusMessage(TEXT("Prompt 不能为空。"), FLinearColor(0.9f, 0.3f, 0.3f, 1.0f));
		return FReply::Handled();
	}

	bIsGenerating = true;
	if (ResponseTextBox.IsValid())
	{
		ResponseTextBox->SetText(FText::GetEmpty());
	}
	SetStatusMessage(TEXT("正在后台加载模型并执行推理，请稍候..."), FLinearColor(0.9f, 0.75f, 0.2f, 1.0f));

	const TWeakPtr<SVPWorkflowIntelligencePanel> PanelWeakPtr = StaticCastSharedRef<SVPWorkflowIntelligencePanel>(AsShared());

	Async(EAsyncExecution::ThreadPool, [PanelWeakPtr, ModelPath, Prompt]()
	{
		FVPWorkflowIntelligenceGenerateResult Result;
		Result.ModelPath = ModelPath;
		Result.Prompt = Prompt;

		const double InitializeStartTime = FPlatformTime::Seconds();
		FLlamaService LlamaService;
		Result.bSucceeded = LlamaService.Initialize(ModelPath);
		Result.InitializeDurationSeconds = FPlatformTime::Seconds() - InitializeStartTime;

		if (Result.bSucceeded)
		{
			const double GenerateStartTime = FPlatformTime::Seconds();
			Result.ResponseText = LlamaService.Generate(Prompt);
			Result.GenerateDurationSeconds = FPlatformTime::Seconds() - GenerateStartTime;
			Result.StatusMessage = FString::Printf(
				TEXT("生成完成。初始化 %.2f s，推理 %.2f s。"),
				Result.InitializeDurationSeconds,
				Result.GenerateDurationSeconds);
		}
		else
		{
			Result.StatusMessage = FString::Printf(
				TEXT("模型初始化失败：%s"),
				*LlamaService.GetLastError());
		}

		LlamaService.Shutdown();

		AsyncTask(ENamedThreads::GameThread, [PanelWeakPtr, Result = MoveTemp(Result)]() mutable
		{
			if (const TSharedPtr<SVPWorkflowIntelligencePanel> PinnedPanel = PanelWeakPtr.Pin())
			{
				PinnedPanel->HandleGenerationCompleted(Result);
			}
		});
	});

	return FReply::Handled();
}

FReply SVPWorkflowIntelligencePanel::HandleClearClicked()
{
	if (PromptTextBox.IsValid())
	{
		PromptTextBox->SetText(FText::GetEmpty());
	}

	if (ResponseTextBox.IsValid())
	{
		ResponseTextBox->SetText(FText::GetEmpty());
	}

	SetStatusMessage(TEXT("已清空输入与输出。"), FLinearColor(0.4f, 0.8f, 1.0f, 1.0f));
	return FReply::Handled();
}

bool SVPWorkflowIntelligencePanel::CanGenerate() const
{
	return !bIsGenerating;
}

FString SVPWorkflowIntelligencePanel::GetModelPath() const
{
	return ModelPathTextBox.IsValid() ? ModelPathTextBox->GetText().ToString().TrimStartAndEnd() : FString();
}

FString SVPWorkflowIntelligencePanel::GetPrompt() const
{
	return PromptTextBox.IsValid() ? PromptTextBox->GetText().ToString().TrimStartAndEnd() : FString();
}

FString SVPWorkflowIntelligencePanel::ResolveInitialModelPath() const
{
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("VPWorkflowIntelligence"));
	if (!Plugin.IsValid())
	{
		return FString();
	}

	const FString ModelsDirectory = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Source/ThirdParty/llama.cpp/models"));
	TArray<FString> FoundModels;
	IFileManager::Get().FindFilesRecursive(FoundModels, *ModelsDirectory, TEXT("*.gguf"), true, false);

	return FoundModels.Num() > 0 ? FoundModels[0] : FString();
}

void SVPWorkflowIntelligencePanel::SetStatusMessage(const FString& InMessage, const FLinearColor& InColor)
{
	StatusColor = FSlateColor(InColor);
	if (StatusTextBlock.IsValid())
	{
		StatusTextBlock->SetText(FText::FromString(InMessage));
		StatusTextBlock->SetColorAndOpacity(StatusColor);
	}
}

void SVPWorkflowIntelligencePanel::HandleGenerationCompleted(const FVPWorkflowIntelligenceGenerateResult& Result)
{
	bIsGenerating = false;

	if (ResponseTextBox.IsValid())
	{
		ResponseTextBox->SetText(FText::FromString(Result.ResponseText));
	}

	SetStatusMessage(
		Result.StatusMessage,
		Result.bSucceeded ? FLinearColor(0.35f, 0.9f, 0.45f, 1.0f) : FLinearColor(0.9f, 0.3f, 0.3f, 1.0f));
}