#include "LlamaService.h"
#include "VPWorkflowIntelligenceLLM.h"

#if WITH_LLAMA_CPP
#include "llama.h"
#endif

#include <string>
#include <vector>

#include "HAL/PlatformFileManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/Paths.h"

namespace
{
	constexpr int32 DefaultMaxGeneratedTokens = 64;
	constexpr int32 DefaultTopK = 40;
	constexpr float DefaultTopP = 0.90f;
	constexpr float DefaultTemperature = 0.70f;

	int32 GetRecommendedThreadCount()
	{
		const int32 HardwareThreads = FPlatformMisc::NumberOfCoresIncludingHyperthreads();
		return FMath::Max(1, HardwareThreads > 0 ? HardwareThreads - 1 : 1);
	}
}

FLlamaService::FLlamaService()
	: Model(nullptr)
	, Context(nullptr)
	, Sampler(nullptr)
	, bInitialized(false)
{
}

FLlamaService::~FLlamaService()
{
	Shutdown();
}

bool FLlamaService::Initialize(const FString& InModelPath)
{
	Shutdown();

	if (InModelPath.IsEmpty())
	{
		SetLastError(TEXT("Model path is empty."));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Warning, TEXT("FLlamaService::Initialize failed: model path is empty."));
		return false;
	}

	ModelPath = FPaths::ConvertRelativePathToFull(InModelPath);
	FPaths::NormalizeFilename(ModelPath);

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*ModelPath))
	{
		SetLastError(FString::Printf(TEXT("Model file not found: %s"), *ModelPath));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Warning, TEXT("FLlamaService::Initialize failed: model file not found: %s"), *ModelPath);
		ModelPath.Reset();
		return false;
	}

	if (!HasBackendSupport())
	{
		SetLastError(TEXT("llama.cpp backend is not linked yet. Headers are configured, but source/static library integration is still pending."));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Warning, TEXT("FLlamaService::Initialize deferred: llama.cpp backend is not linked yet."));
		return false;
	}

#if WITH_LLAMA_CPP
	const FTCHARToUTF8 ModelPathUtf8(*ModelPath);

	llama_backend_init();

	llama_model_params ModelParams = llama_model_default_params();
	ModelParams.n_gpu_layers = 0;
	ModelParams.use_mmap = true;
	ModelParams.use_mlock = false;

	Model = llama_model_load_from_file(ModelPathUtf8.Get(), ModelParams);
	if (Model == nullptr)
	{
		SetLastError(FString::Printf(TEXT("Failed to load GGUF model: %s"), *ModelPath));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Error, TEXT("FLlamaService::Initialize failed to load model: %s"), *ModelPath);
		llama_backend_free();
		ModelPath.Reset();
		return false;
	}

	llama_context_params ContextParams = llama_context_default_params();
	ContextParams.n_ctx = 2048;
	ContextParams.n_batch = 512;
	ContextParams.n_threads = GetRecommendedThreadCount();
	ContextParams.n_threads_batch = GetRecommendedThreadCount();

	Context = llama_init_from_model(Model, ContextParams);
	if (Context == nullptr)
	{
		SetLastError(TEXT("Failed to create llama context from model."));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Error, TEXT("FLlamaService::Initialize failed to create context."));
		llama_model_free(Model);
		Model = nullptr;
		llama_backend_free();
		ModelPath.Reset();
		return false;
	}

	Sampler = llama_sampler_chain_init(llama_sampler_chain_default_params());
	if (Sampler == nullptr)
	{
		SetLastError(TEXT("Failed to create llama sampler chain."));
		UE_LOG(LogVPWorkflowIntelligenceLLM, Error, TEXT("FLlamaService::Initialize failed to create sampler chain."));
		llama_free(Context);
		Context = nullptr;
		llama_model_free(Model);
		Model = nullptr;
		llama_backend_free();
		ModelPath.Reset();
		return false;
	}

	llama_sampler_chain_add(Sampler, llama_sampler_init_top_k(DefaultTopK));
	llama_sampler_chain_add(Sampler, llama_sampler_init_top_p(DefaultTopP, 1));
	llama_sampler_chain_add(Sampler, llama_sampler_init_temp(DefaultTemperature));
	llama_sampler_chain_add(Sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
#endif

	UE_LOG(LogVPWorkflowIntelligenceLLM, Log, TEXT("FLlamaService initialized with model path: %s"), *ModelPath);
	LastError.Reset();
	bInitialized = true;
	return true;
}

FString FLlamaService::Generate(const FString& InPrompt)
{
	if (!bInitialized)
	{
		UE_LOG(LogVPWorkflowIntelligenceLLM, Warning, TEXT("FLlamaService::Generate called before initialization."));
		if (!LastError.IsEmpty())
		{
			return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
		}
		return TEXT("[LLM not initialized]");
	}

	if (InPrompt.IsEmpty())
	{
		return TEXT("");
	}

#if !WITH_LLAMA_CPP
	return FString::Printf(TEXT("[llama backend ready] Prompt accepted but text generation is not wired yet: %s"), *InPrompt);
#else
	if (Model == nullptr || Context == nullptr || Sampler == nullptr)
	{
		SetLastError(TEXT("Generate called without a ready model/context/sampler."));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	const llama_vocab* Vocab = llama_model_get_vocab(Model);
	if (Vocab == nullptr)
	{
		SetLastError(TEXT("Failed to access llama vocabulary."));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	llama_memory_clear(llama_get_memory(Context), true);
	llama_sampler_reset(Sampler);

	const FTCHARToUTF8 PromptUtf8(*InPrompt);
	std::string FormattedPrompt;

	const char* ChatTemplate = llama_model_chat_template(Model, nullptr);
	if (ChatTemplate != nullptr)
	{
		llama_chat_message Message { "user", PromptUtf8.Get() };
		int32 FormattedLength = llama_chat_apply_template(ChatTemplate, &Message, 1, true, nullptr, 0);
		if (FormattedLength < 0)
		{
			SetLastError(TEXT("Failed to apply chat template."));
			return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
		}

		std::vector<char> FormattedBuffer(FormattedLength + 1, '\0');
		FormattedLength = llama_chat_apply_template(ChatTemplate, &Message, 1, true, FormattedBuffer.data(), static_cast<int32>(FormattedBuffer.size()));
		if (FormattedLength < 0)
		{
			SetLastError(TEXT("Failed to apply chat template."));
			return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
		}

		FormattedPrompt.assign(FormattedBuffer.data(), FormattedLength);
	}
	else
	{
		FormattedPrompt.assign(PromptUtf8.Get(), PromptUtf8.Length());
	}

	const int32 PromptTokenCount = -llama_tokenize(Vocab, FormattedPrompt.c_str(), static_cast<int32>(FormattedPrompt.size()), nullptr, 0, true, true);
	if (PromptTokenCount <= 0)
	{
		SetLastError(TEXT("Failed to tokenize prompt."));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	std::vector<llama_token> PromptTokens(PromptTokenCount);
	if (llama_tokenize(Vocab, FormattedPrompt.c_str(), static_cast<int32>(FormattedPrompt.size()), PromptTokens.data(), PromptTokenCount, true, true) < 0)
	{
		SetLastError(TEXT("Failed to tokenize prompt."));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	const int32 ContextWindow = static_cast<int32>(llama_n_ctx(Context));
	if (PromptTokenCount >= ContextWindow)
	{
		SetLastError(FString::Printf(TEXT("Prompt is too long for the current context window (%d >= %d)."), PromptTokenCount, ContextWindow));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	llama_batch Batch = llama_batch_get_one(PromptTokens.data(), PromptTokenCount);
	if (llama_decode(Context, Batch) != 0)
	{
		SetLastError(TEXT("llama_decode failed while evaluating the prompt."));
		return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
	}

	std::string ResponseUtf8;
	for (int32 GeneratedTokenIndex = 0; GeneratedTokenIndex < DefaultMaxGeneratedTokens; ++GeneratedTokenIndex)
	{
		const llama_token NewTokenId = llama_sampler_sample(Sampler, Context, -1);
		if (llama_vocab_is_eog(Vocab, NewTokenId))
		{
			break;
		}

		llama_sampler_accept(Sampler, NewTokenId);

		char PieceBuffer[512] = {};
		const int32 PieceLength = llama_token_to_piece(Vocab, NewTokenId, PieceBuffer, static_cast<int32>(sizeof(PieceBuffer)), 0, true);
		if (PieceLength < 0)
		{
			SetLastError(TEXT("Failed to convert sampled token to text."));
			return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
		}

		ResponseUtf8.append(PieceBuffer, PieceLength);

		llama_token NextToken = NewTokenId;
		Batch = llama_batch_get_one(&NextToken, 1);
		if (llama_decode(Context, Batch) != 0)
		{
			SetLastError(TEXT("llama_decode failed while generating tokens."));
			return FString::Printf(TEXT("[LLM unavailable] %s"), *LastError);
		}

		const int32 UsedContext = static_cast<int32>(llama_memory_seq_pos_max(llama_get_memory(Context), 0)) + 1;
		if (UsedContext >= ContextWindow - 1)
		{
			break;
		}
	}

	LastError.Reset();
	return FString(UTF8_TO_TCHAR(ResponseUtf8.c_str()));
#endif
}

void FLlamaService::Shutdown()
{
	if (bInitialized)
	{
		UE_LOG(LogVPWorkflowIntelligenceLLM, Log, TEXT("FLlamaService shutdown for model path: %s"), *ModelPath);
	}

	if (Sampler != nullptr)
	{
#if WITH_LLAMA_CPP
		llama_sampler_free(Sampler);
#endif
		Sampler = nullptr;
	}

	if (Context != nullptr) 
	{
#if WITH_LLAMA_CPP
		llama_free(Context);
#endif
		Context = nullptr;
	}

	if (Model != nullptr)
	{
#if WITH_LLAMA_CPP
		llama_model_free(Model);
		llama_backend_free();
#endif
		Model = nullptr;
	}

	Model = nullptr;
	bInitialized = false;
	ModelPath.Reset();
}

bool FLlamaService::HasBackendSupport() const
{
#if WITH_LLAMA_CPP
	return true;
#else
	return false;
#endif
}

bool FLlamaService::IsInitialized() const
{
	return bInitialized;
}

const FString& FLlamaService::GetModelPath() const
{
	return ModelPath;
}

const FString& FLlamaService::GetLastError() const
{
	return LastError;
}

void FLlamaService::SetLastError(const FString& InError)
{
	LastError = InError;
}
