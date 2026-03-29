---
title: "Visual Studio & Live Coding with Unreal"
description: "Use Visual Studio (or a similar IDE) with Unreal Editor efficiently: build targets, Live Coding, and when to do a full rebuild instead."
weight: 30
series: ["UE Plugin Development"]
series_order: 2
categories: ["UE Plugin"]
tags: ["unreal-engine", "visual-studio", "live-coding", "plugin", "c++"]
showDate: false
---

You can write plugin code with any editor, but **Unreal + Visual Studio on Windows** is still the path most courses document. This page pairs with [Plugin Development Workflow](plugin-development-workflow/): that one is *what* to do in the project; this one is *how* to stay in flow between **IDE ↔ editor** without rebuilding the universe every time.

## What This Page Solves

**Live Coding** sounds magical—sometimes it is, sometimes it is not. Here is a grounded split: what we use Live Coding for, what we do not, and the habits that kept our group from losing afternoons to “it compiled but nothing changed.”

## 1. Open the Right Workspace

1. Close Unreal if you are regenerating project files (some teams prefer doing this with the editor closed).
2. In your project folder, open **`YourProject.sln`** (or **`.uproject` → right-click → Generate Visual Studio project files** if the solution is missing).
3. In Visual Studio, set the **startup project** to your **game/editor target** (often the project name with **Editor** in the configuration name).

**Student note:** If Visual Studio asks to **retarget** the solution to a newer toolset, coordinate with your course’s recommended VS / VC++ version so everyone matches.

## 2. Pick a Build Configuration You Actually Need

| Configuration | Typical use |
|---------------|-------------|
| **Development Editor** | Day-to-day C++ work in the editor—fast iteration, asserts still available. |
| **Debug Editor** | Slower builds; use when you need deeper debugging and can afford compile time. |
| **Development / Shipping** | More relevant when you package a game; not your first stop for plugin UI experiments. |

When in doubt, **Development Editor** is the default our cohort used for plugin labs.

## 3. What Live Coding Is (and Is Not)

**Live Coding** lets you **recompile C++ while the Unreal Editor stays open**, without closing the whole application. In many setups you trigger it from the editor (**Live Coding** toolbar or shortcut—see your engine version’s docs) or from Visual Studio depending on workflow.

**Good fits:**

- Tweaking function bodies, small logic fixes, and non-structural edits inside existing types.

**Poor fits (expect full rebuild / restart):**

- Adding or removing **UCLASS / USTRUCT / UENUM** members in ways that change reflection layout.
- Changing **`*.Build.cs`** dependencies.
- Editing **`.uplugin`** module lists or load order.
- Large refactors that touch many translation units—sometimes faster to restart clean than to chase odd half-updated state.

**Honest lab experience:** when Live Coding fails mysteriously, **stop and do a full build**, then **restart the editor**. Fighting partial state costs more than the restart.

## 4. A Simple Collaboration Loop (What Worked for Us)

1. Unreal Editor is **running** with your project open.
2. Change a `.cpp` file in Visual Studio and **save**.
3. Trigger **Live Coding** from Unreal (or your configured shortcut).
4. Watch **Output Log** for compile success; fix errors in VS, repeat.
5. Test the feature in the editor immediately.

If two people share a machine or repo: **pull changes → regenerate project files if `Build.cs` changed → full build once**, then return to Live Coding for small steps.

## 5. Visual Studio + Unreal: Small Habits That Help

- **Error List vs Output:** For Unreal, the **build output** often has the real first error; the Error List can be noisy.
- **Clean solution** when you renamed modules or moved files—otherwise stale OBJ/LIB issues show up as bizarre link errors.
- **Rider** and other IDEs work too; the *ideas* (Editor target, Live Coding limits, full rebuild when metadata changes) stay the same.

## 6. What We Learned Along the Way

- **Live Coding is a convenience, not a guarantee.** When the engine says a full rebuild is required, believe it.
- **One change at a time** makes it obvious whether Live Coding or your code caused a regression.
- **Keep the editor log visible**—students often stare at Visual Studio while the useful clue scrolled past in Unreal.

## Further Reading

- [Unreal Engine: Live Coding](https://dev.epicgames.com/documentation/en-us/unreal-engine/live-coding-in-unreal-engine)
- [Unreal Engine: Setting Up Visual Studio for Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine)
