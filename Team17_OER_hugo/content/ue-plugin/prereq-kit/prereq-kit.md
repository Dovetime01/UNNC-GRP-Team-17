---
title: "UE Modules & Plugins"
description: "Prerequisite for plugin work: what Unreal modules and plugins are, how they relate, and how to start building a plugin."
aliases:
  - /ue-plugin/prereq-kit/
weight: 10
categories: ["UE Plugin"]
tags: ["unreal-engine", "plugin", "module", "c++"]
showDate: false
---

Before you write a UE plugin, it helps to separate two ideas: **modules** and **plugins**. Both are ways Unreal organizes C++ (and assets), but they sit at different levels. After this page, you should be able to explain what each is for and follow the basic path **create plugin → author modules → build and enable**.

## 1. What a UE Module Is

A **module** is the **smallest buildable unit of code** in Unreal that can declare dependencies. The engine, the editor, and your game project are all built from many modules.

Think of it as:

- A set of related C++ sources (and optional content) built into a **library** (often a DLL on Windows; linked and loaded as part of the editor or game).
- Each module declares its name, which other modules it needs, and whether it is editor-only or also loaded at runtime—via a **`*.Build.cs`** file.

### Why modules exist

- **Load what you need**: Only load features you use, which helps startup time and footprint.
- **Clear dependencies**: Use `PublicDependencyModuleNames` / `PrivateDependencyModuleNames` in `Build.cs` instead of ad-hoc includes everywhere.
- **Clear boundaries**: Keep “editor-only tools” separate from “shipped game logic” so editor code does not end up in a shipping build by mistake.

### Common module types (names are enough for now)

| Type | Rough meaning |
|------|----------------|
| **Runtime** | Loaded when the game runs; plugins that must work in-game usually have a Runtime module. |
| **Editor** | Editor-only: custom windows, importers, level tools, and so on. |
| **Developer / UncookedOnly** | Used in development or specific stages; for intro work, just know the split exists. |

The solution you open in **Visual Studio** or **Rider** usually maps to one or more modules; after you change code you **compile** the right target (Editor or Game) for those changes to apply.

---

## 2. What a UE Plugin Is

A **plugin** is a **larger package** than a module: an **optional feature bundle** that may contain:

- One or more **C++ modules**;
- **Content** (Blueprints, materials, levels, etc.);
- Sometimes **Python, config, third-party libraries**, and more.

At the plugin root there is a **`*.uplugin`** file (JSON) that describes:

- Name, version, supported engine version;
- Which **modules** it contains and whether they are enabled by default;
- Categories (Editor vs Runtime, etc.).

### Typical folders and files at the plugin root

A plugin folder contains several standard directories plus the manifest. The figure below shows one example layout; your plugin folder name will match the plugin you created.

<!-- Place your screenshot at static/ue-plugin/plugin-folder-structure.png (served as /ue-plugin/plugin-folder-structure.png). -->

![Unreal Engine plugin root: Binaries, Config, Content, Intermediate, Resources, Source, and the .uplugin file](/ue-plugin/plugin-folder-structure.pic.jpg)

- **Binaries** — Output of compilation: compiled binaries (for example `.dll` files on Windows) for your plugin’s modules.
- **Config** — Optional plugin-specific configuration (defaults and settings scoped to this plugin).
- **Content** — Unreal assets that ship with the plugin (Blueprints, materials, meshes, and other `/Game`-style content owned by the plugin).
- **Intermediate** — Temporary files produced while building; can usually be deleted and will be regenerated on the next compile.
- **Resources** — Non-asset files the editor uses with the plugin, commonly **icons and images** for UI or the Plugins browser.
- **Source** — C++ source for each module, including each module’s **`*.Build.cs`** file (see below).
- **`<PluginName>.uplugin`** — The plugin **manifest** in JSON: modules, versioning, loading rules, and metadata the editor reads before your code runs.

#### `Build.cs` (under `Source`)

Each **`*.Build.cs`** file defines a class derived from **`ModuleRules`**. In the constructor you set properties (such as dependency module names) that tell **Unreal Build Tool (UBT)** how to compile that module. These files use **C# syntax**; UBT reads them to build the correct compile environment for your plugin modules.

### Plugins vs your **project**

- The **project** is the overall container for your game or tool.
- **Plugins** live under the project (or under the engine) and are toggled under **Edit → Plugins**; when enabled, their modules are loaded and compiled according to the rules in the `.uplugin` file.

So **plugin development usually means “create a plugin and put your modules inside it”**, rather than putting everything in the project’s `Source/MyGame` folder. For learning you can still change the game module first; when you want reuse and distribution, you typically move that work into a plugin.

---

## 3. How modules and plugins work together

In one sentence: **the plugin is the box; the modules are the engines inside the box.**

- A plugin has **at least one** module (many small plugins have a single Runtime or Editor module).
- Larger plugins may split **runtime logic** and **editor UI** into separate modules so the Editor module is not needed in a packaged game.

A practical mental model:

1. **`.uplugin`**: Declares “this plugin exists, which modules it has, and whether they are on by default.”
2. **Each module’s `*.Build.cs`**: Tells the build which engine modules you depend on (for example `Core`, `Engine`, `Slate`).
3. **Source code**: Lives under that module’s `Public` / `Private` folders and implements behavior with Unreal’s module APIs.

---

## 4. Using them to **develop a plugin** (workflow overview)

Below is a **beginner-friendly order**; check the docs for your exact UE version for menu names and details.

### 4.1 Create a plugin from the editor

1. **Edit → Plugins → Add → New Plugin** (or the equivalent entry in your version).
2. Pick a template (for example **Blank** or a starter with sample code), then set the plugin name and path.
3. After generation, under `Plugins/YourPluginName/` you should see the `.uplugin` file and module folders under `Source/`.

### 4.2 Check modules and `Build.cs`

- Open `Source/ModuleName/ModuleName.Build.cs` and confirm `PublicDependencyModuleNames` lists the engine modules you need (for editor UI tools, `Slate`, `SlateCore`, `EditorStyle`, etc., are common).
- New `.cpp` / `.h` files usually go under that module’s `Private` or `Public`, with correct module export macros (`MODULE_API`, etc.) as your course goes deeper.

### 4.3 Build

- Regenerate project files (**Generate Visual Studio project files**, or the Rider flow), then build the **Editor** target.
- When the build succeeds, enable the plugin in the editor and restart if prompted; your menu entries or windows should appear.

### 4.4 Common pitfalls (good to know early)

- After changing `Build.cs` or `.uplugin`, you often need to **regenerate project files** before building again.
- Keep **Editor module** code out of pure Runtime paths that ship in the game, and vice versa—the split should match the module types in the `.uplugin` file.

---

## 5. Summary

| Concept | Role | What to remember first |
|--------|------|-------------------------|
| **Module** | Buildable unit with explicit dependencies | `Build.cs` + source folders; this is where you write C++. |
| **Plugin** | Optional bundle; may contain several modules and content | `.uplugin` file; enable under the Plugins panel. |
| **Relationship** | Plugins contain modules | Building a plugin = create or edit modules inside the plugin, then build and enable. |

If next year’s students remember only one line: **open the `.uplugin` to see which modules exist, open each module’s `Build.cs` for dependencies, then edit source and build**—that is enough to continue with labs and official docs.

## Further Reading

- [Unreal Engine: An Overview of Plugins](https://dev.epicgames.com/documentation/en-us/unreal-engine/plugins-in-unreal-engine)
- [Unreal Engine: Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
