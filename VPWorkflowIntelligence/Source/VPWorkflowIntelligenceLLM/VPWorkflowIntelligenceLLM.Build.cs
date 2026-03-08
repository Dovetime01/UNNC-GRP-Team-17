using UnrealBuildTool;
using System.IO;

public class VPWorkflowIntelligenceLLM : ModuleRules
{
	public VPWorkflowIntelligenceLLM(ReadOnlyTargetRules Target) : base(Target)
	{
		string PluginSourceDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, ".."));
		string ThirdPartyDirectory = Path.Combine(PluginSourceDirectory, "ThirdParty");
		string LlamaCppRoot = Path.Combine(ThirdPartyDirectory, "llama.cpp");
		string LlamaCppInclude = Path.Combine(LlamaCppRoot, "include");
		string LlamaCppSrc = Path.Combine(LlamaCppRoot, "src");
		string GgmlRoot = Path.Combine(LlamaCppRoot, "ggml");
		string GgmlInclude = Path.Combine(GgmlRoot, "include");
		string GgmlSrc = Path.Combine(GgmlRoot, "src");
		string GgmlCpuSrc = Path.Combine(GgmlSrc, "ggml-cpu");
		string LlamaBuildRoot = Path.Combine(LlamaCppRoot, "build");
		string LlamaReleaseLibDir = Path.Combine(LlamaBuildRoot, "src", "Release");
		string GgmlReleaseLibDir = Path.Combine(LlamaBuildRoot, "ggml", "src", "Release");
		string[] RequiredLibraries = new string[]
		{
			Path.Combine(LlamaReleaseLibDir, "llama.lib"),
			Path.Combine(GgmlReleaseLibDir, "ggml.lib"),
			Path.Combine(GgmlReleaseLibDir, "ggml-base.lib"),
			Path.Combine(GgmlReleaseLibDir, "ggml-cpu.lib"),
		};
		bool bHasLlamaPrebuiltLibraries = true;

		foreach (string LibraryPath in RequiredLibraries)
		{
			if (!File.Exists(LibraryPath))
			{
				bHasLlamaPrebuiltLibraries = false;
				break;
			}
		}

		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Public"),
			}
		);

		PrivateIncludePaths.AddRange(
			new string[]
			{
				Path.Combine(ModuleDirectory, "Private"),
				LlamaCppSrc,
				GgmlSrc,
				GgmlCpuSrc,
			}
		);

		PublicSystemIncludePaths.AddRange(
			new string[]
			{
				LlamaCppInclude,
				GgmlInclude,
				LlamaCppSrc,
				GgmlSrc,
				GgmlCpuSrc,
			}
		);

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"ApplicationCore",
				"Projects",
			}
		);

		PublicDefinitions.AddRange(
			new string[]
			{
				string.Format("WITH_LLAMA_CPP={0}", bHasLlamaPrebuiltLibraries ? 1 : 0),
				"GGML_USE_CPU",
				"GGML_USE_CPU_REPACK",
				"_CRT_SECURE_NO_WARNINGS",
			}
		);

		if (Target.Platform == UnrealTargetPlatform.Win64 && bHasLlamaPrebuiltLibraries)
		{
			PublicAdditionalLibraries.AddRange(RequiredLibraries);
			PublicSystemLibraries.AddRange(
				new string[]
				{
					"Advapi32.lib",
					"User32.lib",
					"Shell32.lib",
				}
			);
		}
	}
}
