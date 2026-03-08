#include "VPWorkflowIntelligenceLLM.h"

DEFINE_LOG_CATEGORY(LogVPWorkflowIntelligenceLLM);

#define LOCTEXT_NAMESPACE "FVPWorkflowIntelligenceLLMModule"

void FVPWorkflowIntelligenceLLMModule::StartupModule()
{
	UE_LOG(LogVPWorkflowIntelligenceLLM, Log, TEXT("VPWorkflowIntelligenceLLM module started"));
}

void FVPWorkflowIntelligenceLLMModule::ShutdownModule()
{
	UE_LOG(LogVPWorkflowIntelligenceLLM, Log, TEXT("VPWorkflowIntelligenceLLM module shutdown"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FVPWorkflowIntelligenceLLMModule, VPWorkflowIntelligenceLLM)
