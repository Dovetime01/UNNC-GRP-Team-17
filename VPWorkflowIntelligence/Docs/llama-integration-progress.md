# VPWorkflowIntelligence + llama.cpp 集成进度

更新时间：2026-03-07

## 1. 当前项目结构检查结果

### 已确认现状
- 插件根目录存在 `VPWorkflowIntelligence.uplugin`
- 当前插件模块为 `VPWorkflowIntelligence`
- 插件模块类型为 `Editor`
- 已存在基础菜单/按钮入口
- `llama.cpp` 已下载到 `Source/ThirdParty/llama.cpp`

### 当前模块状态
- 主模块构建文件：`Source/VPWorkflowIntelligence/VPWorkflowIntelligence.Build.cs`
- 主模块源码目录：`Source/VPWorkflowIntelligence/Private`
- 主模块头文件目录：`Source/VPWorkflowIntelligence/Public`

### llama.cpp 目录状态
已确认存在以下关键目录：
- `Source/ThirdParty/llama.cpp/include`
- `Source/ThirdParty/llama.cpp/src`
- `Source/ThirdParty/llama.cpp/ggml`
- `Source/ThirdParty/llama.cpp/ggml/src`

已确认公共头：
- `llama.h`
- `llama-cpp.h`

## 2. 结构评估结论

当前结构适合做“UE 编辑器内本地 AI Agent”，但还没有进入“可编译接入 llama.cpp”的阶段。

### 优点
1. 插件已经能在编辑器中加载
2. 已有 UI/菜单入口，适合作为 Agent 面板入口
3. `llama.cpp` 已经放在插件 `Source/ThirdParty` 下，路径方向正确

### 当前缺口
1. 还没有独立的第三方桥接模块
2. `Build.cs` 里还没有配置 `llama.cpp` include path
3. 还没有配置 `ggml` / `llama` 所需源码或静态库
4. 还没有 UE 侧推理服务封装
5. 还没有异步推理线程方案
6. 当前插件是 `Editor` 模块，尚不适用于 Runtime 分发

## 3. 推荐实施方式

不建议直接把大量 `llama.cpp` 源码逻辑塞进当前 `VPWorkflowIntelligence` 模块。

### 推荐拆分
#### 模块 A：`VPWorkflowIntelligence`
职责：
- 菜单
- 面板
- Editor Utility Widget
- 用户交互
- 推理请求发起

#### 模块 B：建议新增 `VPWorkflowIntelligenceLLM`
职责：
- 封装 `llama.cpp`
- 模型加载
- context 创建
- prompt 推理
- token 流式回调
- 资源释放
- 后台线程调度

#### 第三方目录：`Source/ThirdParty/llama.cpp`
职责：
- 保持原始第三方源码
- 尽量少改上游文件

## 4. 建议的集成策略

### 第一阶段：CPU Only
先只做 CPU 版本，不接 CUDA/Vulkan。

原因：
- Unreal Build Tool 集成更简单
- 更容易先打通最小闭环
- 便于验证插件内推理体验

### 第二阶段：最小可运行 POC
目标：
- 在 UE 插件中加载一个 GGUF 模型
- 输入一段 prompt
- 返回完整文本结果
- 将结果打印到日志或 UI

### 第三阶段：Agent 化
目标：
- 增加 system prompt
- 增加上下文记忆
- 增加工具调用接口
- 与 VP 工作流联动

## 5. 当前最优先的下一步

### 下一步 1：新增 LLM 桥接模块
建议新增：
- `Source/VPWorkflowIntelligenceLLM/VPWorkflowIntelligenceLLM.Build.cs`
- `Source/VPWorkflowIntelligenceLLM/Public/...`
- `Source/VPWorkflowIntelligenceLLM/Private/...`

### 下一步 2：在桥接模块内接入 llama.cpp
优先方案：
- 只接 `llama.cpp` + `ggml` CPU 路径
- 先做最小源码编译接入

### 下一步 3：暴露一个 UE 友好的服务类
建议接口形态：
- `Initialize(ModelPath)`
- `Generate(Prompt)`
- `Shutdown()`

## 6. 风险记录

### 构建风险
`llama.cpp` 上游源码体量较大，直接纳入 UBT 时可能遇到：
- Windows/MSVC 编译选项兼容问题
- OpenMP 依赖问题
- 某些后端源码不适合直接纳入
- 编译时间较长

### 运行风险
- 模型文件较大
- 编辑器内存压力明显
- 推理不能阻塞游戏线程/编辑器主线程

## 7. 当前判断

该方案可行，且适合作为“本地、低延迟、无外部服务依赖”的虚拟制作 AI 插件方案。

但最佳实践是：
- 保持主插件模块轻量
- 新增独立 LLM 模块
- 先 CPU-only 跑通
- 再扩展到更完整的 Agent 能力

## 8. 下一次操作计划

下一个实施动作建议为：
1. 新增 `VPWorkflowIntelligenceLLM` 模块骨架
2. 更新 `.uplugin` 注册新模块
3. 修改主模块 `Build.cs` 依赖新模块
4. 增加最小 `FLlamaService` 封装骨架

## 9. 已完成的骨架搭建

本轮已完成：
- 新增 `Source/VPWorkflowIntelligenceLLM/VPWorkflowIntelligenceLLM.Build.cs`
- 新增 `Source/VPWorkflowIntelligenceLLM/Public/VPWorkflowIntelligenceLLM.h`
- 新增 `Source/VPWorkflowIntelligenceLLM/Public/LlamaService.h`
- 新增 `Source/VPWorkflowIntelligenceLLM/Private/VPWorkflowIntelligenceLLM.cpp`
- 新增 `Source/VPWorkflowIntelligenceLLM/Private/LlamaService.cpp`
- 在 `.uplugin` 中注册 `VPWorkflowIntelligenceLLM` 模块
- 在现有编辑器模块中加入对 `VPWorkflowIntelligenceLLM` 的依赖

当前状态：
- 已完成模块级骨架搭建
- 已具备后续接入 `llama.cpp` 的落点
- `FLlamaService` 当前仍为占位实现，尚未真正调用 `llama.cpp`

## 10. 下一步实施目标

下一步将进入：
1. 在 `VPWorkflowIntelligenceLLM` 中配置 `llama.cpp` 和 `ggml` 的 include/source
2. 先做 CPU-only 最小可编译接入
3. 用真实 `llama.cpp` 初始化逻辑替换 `FLlamaService` 占位实现

## 11. 本轮新增进展（预集成阶段）

本轮已完成：
- 在 `VPWorkflowIntelligenceLLM.Build.cs` 中加入 `llama.cpp` / `ggml` 头文件路径配置
- 为后续接入预留编译开关：`WITH_LLAMA_CPP`
- 在 `FLlamaService` 中加入：
	- 模型文件路径规范化与存在性校验
	- `LastError` 错误状态
	- `HasBackendSupport()` 能力探测接口
	- `llama_model` / `llama_context` / `llama_sampler` 句柄落点

当前状态说明：
- **头文件层接入已完成**
- **真实 backend 仍未接入**
- 当前 `WITH_LLAMA_CPP=0`，表示还未把 `llama.cpp` 源码或静态库真正编译进 UBT

这样做的目的：
- 先把 UE 侧封装边界稳定下来
- 避免在尚未验证源码编译策略前，把业务代码和第三方源码耦死
- 为下一步 CPU-only 接入提供明确落点

## 12. 当前阻塞项

当前环境缺少以下本地构建工具，导致无法在此轮直接验证真实编译：
- `msbuild`
- `.NET SDK`
- `cmake`
- `cl`
- `ninja`

因此本轮只能完成“预集成”和“接口收口”，还不能在当前环境里完成 `llama.cpp` 静态库预编译或源码编译验证。

## 13. 下一步的具体实施策略

优先推荐两种路径二选一：

### 路径 A：预编译静态库接入（更稳）
1. 用 CMake/VS 单独编译 `llama.cpp` CPU-only 静态库
2. 在 UE 模块里通过 `PublicAdditionalLibraries` 链接
3. 将 `WITH_LLAMA_CPP` 切到 `1`
4. 在 `FLlamaService` 中接入真实 `llama_backend_init()` / `llama_model_load_from_file()`

### 路径 B：源码直接并入 UBT（更激进）
1. 在模块中建立 `ggml-base` / `ggml-cpu` / `llama` 的源码并入策略
2. 处理 `.c` / `.cpp`、架构宏、OpenMP、MSVC 指令集参数
3. 再切换 `WITH_LLAMA_CPP=1`

对你的项目，**建议优先走路径 A**。

## 14. 对 `llama.cpp/build` 的检查结果

已检查 `Source/ThirdParty/llama.cpp/build`，当前判断如下：

### 已确认内容
- 已成功生成 Visual Studio 工程
- 生成器为 `Visual Studio 17 2022`
- 当前是多配置工程：`Debug;Release;MinSizeRel;RelWithDebInfo`
- 当前配置为 `BUILD_SHARED_LIBS=ON`
- 当前配置为 `GGML_CPU=ON`
- 当前配置为 `GGML_OPENMP=ON`
- 当前配置为 `GGML_NATIVE=ON`
- 当前配置里 `GGML_AVX2=OFF`

### 关键判断
从目录状态看，**目前更像是已经完成了 CMake Configure/Generate，但还没有真正完成一次目标构建**。

依据：
- `build/src/llama.vcxproj`、`build/ggml/src/ggml.vcxproj` 等工程文件已经生成
- 这些工程的输出目录被设置为 `build/bin/Release`、`build/bin/Debug` 等
- 但当前没有发现：
	- `build/bin/...` 目录
	- `llama.dll` / `llama.lib`
	- `ggml.dll` / `ggml.lib`
	- `ggml-cpu.dll` / `ggml-cpu.lib`
	- `.obj` / `.tlog` / `.lastbuildstate`

这通常说明：
- 你已经在 `cmake-gui` 里完成了 **Configure + Generate**
- 但还没有点 **Build**，或者 Build 没有实际产出到当前配置目录

## 15. 当前构建配置对 UE 的影响

当前生成配置并不是最理想的 UE 首次接入配置：

### 1. `BUILD_SHARED_LIBS=ON`
这会优先生成 DLL 方案。

影响：
- UE 里可以接，但需要同时管理 `.dll` 和 `.lib`
- 插件分发、加载路径、运行时依赖会更复杂

### 2. `GGML_OPENMP=ON`
这可能引入额外 OpenMP 运行时依赖。

影响：
- 初次接入更容易遇到运行时缺库问题
- 不利于先打通最小闭环

### 3. `GGML_NATIVE=ON`
这会针对当前机器做本机优化。

影响：
- 在你本机测试时性能更高
- 但如果插件要迁移到别的机器，兼容性会变差

## 16. 推荐的下一次 CMake 编译参数

如果你准备重新编一次，更推荐先用下面这组思路：

- `BUILD_SHARED_LIBS=OFF`
- `GGML_CPU=ON`
- `GGML_OPENMP=OFF`
- `GGML_NATIVE=OFF`
- `GGML_CUDA=OFF`
- `GGML_VULKAN=OFF`

目标：
- 先得到最干净的 CPU-only 静态库
- 先让 UE 成功链接
- 然后再逐步恢复 OpenMP 或更激进优化

## 17. 对当前状态的结论

可以继续检查，但按目前目录内容，**还没有可直接给 UE 链接的产物**。

当前最可能需要的是：
1. 在 `cmake-gui` 或 Visual Studio 里真正执行一次 `Release|x64` 构建
2. 确认 `build/bin/Release` 下是否出现：
	 - `llama.dll` / `llama.lib`
	 - `ggml.dll` / `ggml.lib`
	 - `ggml-base.dll` / `ggml-base.lib`
	 - `ggml-cpu.dll` / `ggml-cpu.lib`

只要这些产物出来，就可以进入 UE 侧链接阶段。

## 18. 二次检查结果：已发现可用产物

在你完成 build 之后，现已确认 `Release` 产物存在。

### 已确认的核心库
- [Source/ThirdParty/llama.cpp/build/src/Release/llama.lib](Source/ThirdParty/llama.cpp/build/src/Release/llama.lib)
- [Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml.lib](Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml.lib)
- [Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml-base.lib](Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml-base.lib)
- [Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml-cpu.lib](Source/ThirdParty/llama.cpp/build/ggml/src/Release/ggml-cpu.lib)

### 已确认的辅助库
- [Source/ThirdParty/llama.cpp/build/common/Release/common.lib](Source/ThirdParty/llama.cpp/build/common/Release/common.lib)
- [Source/ThirdParty/llama.cpp/build/tools/mtmd/Release/mtmd.lib](Source/ThirdParty/llama.cpp/build/tools/mtmd/Release/mtmd.lib)
- [Source/ThirdParty/llama.cpp/build/tools/server/Release/server-context.lib](Source/ThirdParty/llama.cpp/build/tools/server/Release/server-context.lib)

### 已确认的可执行文件
`build/bin/Release` 下已有大量工具与测试程序，例如：
- [Source/ThirdParty/llama.cpp/build/bin/Release/llama-cli.exe](Source/ThirdParty/llama.cpp/build/bin/Release/llama-cli.exe)
- [Source/ThirdParty/llama.cpp/build/bin/Release/llama-server.exe](Source/ThirdParty/llama.cpp/build/bin/Release/llama-server.exe)
- [Source/ThirdParty/llama.cpp/build/bin/Release/llama-quantize.exe](Source/ThirdParty/llama.cpp/build/bin/Release/llama-quantize.exe)

### 额外判断
- 当前**没有发现 `.dll`**
- 当前生成物更适合按 **`.lib` 预编译库接入** UE
- 这对插件首轮接入是好事，路径会比 DLL 方案更简单

### 对 UE 集成的意义
这说明我们现在已经具备进入下一阶段的前提：
1. 在 `VPWorkflowIntelligenceLLM.Build.cs` 中链接这些 `Release` 库
2. 将 `WITH_LLAMA_CPP` 切换到 `1`
3. 在 `FLlamaService` 中接入真实 `llama_backend_init()` / `llama_model_load_from_file()`

## 19. 本轮新增进展：已接入预编译库

本轮已完成：
- 在 `VPWorkflowIntelligenceLLM.Build.cs` 中自动检查以下库是否存在：
	- `llama.lib`
	- `ggml.lib`
	- `ggml-base.lib`
	- `ggml-cpu.lib`
- 当库文件存在时，自动将 `WITH_LLAMA_CPP` 切换为 `1`
- 在 Win64 下将上述 Release 库加入 UE 链接列表

## 20. 本轮新增进展：已接入真实初始化链路

`FLlamaService::Initialize()` 已从“占位实现”升级为“真实加载流程”：
- 调用 `llama_backend_init()`
- 调用 `llama_model_default_params()`
- 调用 `llama_model_load_from_file()`
- 调用 `llama_context_default_params()`
- 调用 `llama_init_from_model()`

`FLlamaService::Shutdown()` 已接入真实释放流程：
- `llama_free()`
- `llama_model_free()`
- `llama_backend_free()`

## 21. 当前状态

当前已经进入：
- **UE 侧预编译库链接阶段**
- **真实模型初始化阶段**

尚未完成：
- 真正的 tokenization / decode / sampling 文本生成
- UI 层输入输出联动
- 异步线程化推理

目前 `Generate()` 仍是过渡实现：
- 如果模型未初始化，会返回明确错误
- 如果模型已初始化，会返回“backend ready”的占位响应

## 22. 初始化验证入口已接入

为了直接验证 `FLlamaService::Initialize()`，已在插件按钮入口加入一次实测调用：
- 入口位置：[Source/VPWorkflowIntelligence/Private/VPWorkflowIntelligence.cpp](Source/VPWorkflowIntelligence/Private/VPWorkflowIntelligence.cpp)
- 调用方式：点击插件按钮时，先执行一次 `FLlamaService::Initialize()`
- 当前测试模型路径：
	- `C:/Users/dylon/Documents/Unreal Projects/Pico/Plugins/VPWorkflowIntelligence/Source/ThirdParty/llama.cpp/models/Qwen3.5-4B-Q4_K_M (1).gguf`

当前按钮行为：
1. 先尝试加载该 GGUF 模型
2. 成功则在屏幕上输出绿色成功消息，并继续原有按钮逻辑
3. 失败则在屏幕上输出红色错误消息，并停止后续逻辑

这样你现在可以直接在 UE 编辑器里点击插件按钮，验证 `Initialize()` 是否能成功载入模型。

## 23. 初始化验证结果：成功

已在 UE 内成功完成一次真实初始化验证。

### 实际日志
- `LogVPWorkflowIntelligenceLLM: FLlamaService initialized with model path: C:/Users/dylon/Documents/Unreal Projects/Pico/Plugins/VPWorkflowIntelligence/Source/ThirdParty/llama.cpp/models/Qwen3.5-4B-Q4_K_M (1).gguf`
- `LogTemp: FLlamaService::Initialize succeeded in 0.54 s. Model: C:/Users/dylon/Documents/Unreal Projects/Pico/Plugins/VPWorkflowIntelligence/Source/ThirdParty/llama.cpp/models/Qwen3.5-4B-Q4_K_M (1).gguf`
- `LogVPWorkflowIntelligenceLLM: FLlamaService shutdown for model path: C:/Users/dylon/Documents/Unreal Projects/Pico/Plugins/VPWorkflowIntelligence/Source/ThirdParty/llama.cpp/models/Qwen3.5-4B-Q4_K_M (1).gguf`

### 结论
- `llama.cpp` 预编译库已被 UE 模块正确链接
- `FLlamaService::Initialize()` 已能成功加载本地 GGUF 模型
- `llama_context` 创建成功
- `Shutdown()` 释放链路正常

这意味着当前插件已经成功跨过最关键的一步：
**UE 插件内原生加载本地 GGUF 模型已验证通过。**

## 24. 下一步建议

接下来应优先进入：
1. 实现真正的 `Generate()`
2. 接入 tokenization / decode / sampling
3. 将结果回传到 UE UI
4. 再做异步线程化，避免阻塞主线程

## 25. 本轮新增进展：`Generate()` 已接入最小推理链路

本轮已将 `FLlamaService::Generate()` 从占位实现升级为最小可运行推理流程，当前已包含：
- prompt 文本预处理
- 优先使用模型自带 chat template
- `llama_tokenize()`
- `llama_decode()`
- sampler chain 采样
- `llama_token_to_piece()` 拼接输出文本
- `llama_memory_clear()` 在每次生成前重置上下文

当前 sampler 配置为：
- `top_k = 40`
- `top_p = 0.90`
- `temperature = 0.70`
- 最终采样器：`dist`

最大生成 token 数当前为：
- `128`

## 26. 按钮测试入口已扩展到 Generate

当前按钮点击流程已升级为：
1. `Initialize()` 加载模型
2. 调用 `Generate()` 执行一次真实推理
3. 将结果写入 Output Log
4. 最后执行 `Shutdown()`

当前测试 prompt 为：
- `请用一句简短的话介绍你自己。请用中文回答。`

因此现在在 UE 中点击插件按钮，已经不仅能验证模型载入，还能验证最小文本生成链路。

---

如果继续推进，下一步应直接进入“流式输出与异步推理接入”。