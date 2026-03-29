---
title: "Plugin Development Workflow"
description: "A practical loop for building Unreal Engine plugins—from creating a plugin to enabling it, iterating on code, and testing in the editor."
weight: 20
series: ["UE Plugin Development"]
series_order: 1
categories: ["UE Plugin"]
tags: ["unreal-engine", "plugin", "workflow", "c++"]
showDate: false
---

This page is written from a **student lab mindset**: you already know what [modules and plugins](../prereq-kit/prereq-kit/) are. Here we focus on **what you actually do day to day** when the plugin exists—so you spend less time guessing “what should I click next?”

## What This Page Solves

Unreal’s documentation is deep, but it rarely hands you one **repeatable loop**. The workflow below is the one our cohort kept coming back to after trial and error: it is not the only valid path, but it is a reliable default for coursework and small tools.

## 1. Start From a Small, Testable Goal

Before touching **New Plugin**, write one sentence: *“When I press X, the editor should do Y.”* Examples:

- Add a menu command that prints a message to the Output Log.
- Spawn a test actor when a button is clicked.

A narrow goal keeps your first `Build.cs` dependencies small and makes failures easier to read.

## 2. Create the Plugin (Editor-First)

1. **Edit → Plugins → Add → New Plugin** (wording may vary slightly by engine version).
2. Pick a template (**Blank** is fine for learning).
3. Choose a **short, PascalCase-style name** you will not regret in file paths.
4. After generation, find your plugin under `YourProject/Plugins/<PluginName>/`.

**Student tip:** If your team shares a repo, commit the whole `Plugins/<PluginName>` folder so everyone gets the same `.uplugin` and `Source/` tree. Missing only the `.uplugin` while the code exists is a common “it works on my machine” trap.

## 3. Enable It and Let the Editor Know

1. **Edit → Plugins**, search for your plugin, enable it, and **restart** if Unreal asks.
2. If the editor does not see new modules, confirm the `.uplugin` lists the modules you expect and that **EnabledByDefault** (or your enable state) matches what you want.

If something feels “invisible,” check the **Output Log** and **Message Log** first—students often assume silence means success.

## 4. The Core Iteration Loop (Code)

Once the plugin skeleton builds:

1. Open your **IDE** from the project (see the companion page on **Visual Studio + Live Coding**).
2. Edit `.h` / `.cpp` in the correct **module** folder.
3. **Compile** (Live Coding for small changes, full build when the engine tells you to—details in the next article).
4. Return to Unreal and **test** (menus, buttons, PIE, or your feature’s entry point).

**Rule of thumb we adopted in class:** if you changed **only** logic inside existing classes, Live Coding is usually enough. If you changed **class layout**, **reflection macros**, **`Build.cs`**, or **`.uplugin`**, plan for a **full editor rebuild** and often a **restart**.

## 5. Test Like You Mean It

- Use **Play In Editor (PIE)** when your feature must behave in a running level.
- Use **editor-only** tests (menus, asset actions) without PIE when that is enough.
- Keep **one simple level** or empty map for “does it crash?” checks—heavy scenes make it harder to see whether your plugin or the content caused the issue.

## 6. When Things Go Wrong (Lightweight Troubleshooting)

| Symptom | Something to check |
|--------|---------------------|
| Compile errors in the IDE | Module name typos, missing `MODULE_API`, wrong `#include` paths. |
| Link errors | Missing module in `PublicDependencyModuleNames` / `PrivateDependencyModuleNames`. |
| Editor loads but feature missing | Plugin disabled, module not loaded, or code path never registered (e.g. missing module startup). |
| Strange behavior only after packaging | You may be testing **Editor** code paths; confirm **Runtime** vs **Editor** module split. |

## 7. What We Learned Along the Way

- **Smallest vertical slice first** beats a big design doc: get one button or one log line working, then grow.
- **Rename early, not late**—plugin and module names propagate through paths and build scripts.
- **Comment your intent** in `Build.cs` when you add a dependency (“needed for Slate menu”), so your future self (or a teammate) knows why it is there.

## Further Reading

- [Unreal Engine: Creating a Plugin](https://dev.epicgames.com/documentation/en-us/unreal-engine/creating-a-plugin-in-unreal-engine)
- [Unreal Engine: Plugin Basics](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-plugin-basics)
