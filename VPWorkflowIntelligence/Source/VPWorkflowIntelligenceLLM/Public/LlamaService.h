#pragma once

#include "CoreMinimal.h"

struct llama_context;
struct llama_model;
struct llama_sampler;

class VPWORKFLOWINTELLIGENCELLM_API FLlamaService
{
public:
	FLlamaService();
	~FLlamaService();

	bool Initialize(const FString& InModelPath);
	FString Generate(const FString& InPrompt);
	void Shutdown();

	bool HasBackendSupport() const;
	bool IsInitialized() const;
	const FString& GetModelPath() const;
	const FString& GetLastError() const;

private:
	void SetLastError(const FString& InError);

	FString ModelPath;
	FString LastError;
	llama_model* Model;
	llama_context* Context;
	llama_sampler* Sampler;
	bool bInitialized;
};
