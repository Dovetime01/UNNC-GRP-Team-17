---
title: "Level Snapshot Foundations"
description: "Use Level Snapshots as a clean packaging unit for previs scene work."
weight: 10
series: ["Scene Packaging Foundations"]
series_order: 2
categories: ["Scene Packaging"]
tags: ["level-snapshot", "naming", "handoff"]
showDate: false
---

## What This Solves

In previs work, scenes change quickly. Without a packaging rule, it becomes hard to tell which scene state is reviewable, which one is stable enough to share, and which one should be restored later.

## Recommended Packaging Unit

Treat each virtual scene package as a Level Snapshot tied to a specific scene and shot context.

Use this naming pattern:

```text
Scene_xx_Shot_xx
```

This keeps snapshot naming aligned with shot-based work rather than vague version labels.

## How to Package a Level

Unreal Engine provides the **Levels** panel as the primary interface for managing level-based scene packaging. The workflow below describes the standard process for creating, saving, and sharing a packaged scene state as a Level Snapshot.

### Step 1 — Open the Levels Panel

Select **Window → Level** from the menu bar. The Levels panel appears docked at the bottom of the screen by default. This panel lists all sub-levels currently loaded in the world and allows you to manage their visibility, locking, and save state.

### Step 2 — Organize Your Levels

Before packaging, confirm that the scene hierarchy in the Levels panel reflects the intended shot context. Common practice is to keep persistent layout levels separate from shot-specific content levels:

- **Persistent / Layout Level** — contains world geometry, grid, and shared infrastructure.
- **Shot Levels** — contain actors, lighting, and camera logic specific to a given shot or scene.

Actors can be moved between levels by selecting them in the viewport or World Outliner and using the **Move Selected Actors to Level** command (accessible via right-click in the World Outliner).

### Step 3 — Save the Level

With the correct actors assigned to the appropriate sub-level, save each level individually:

1. In the Levels panel, right-click the level you wish to save.
2. Select **Save**.
3. Alternatively, use **File → Save Current** to save the active level directly.

Save frequently during previs work. An unsaved level state is lost if the editor closes unexpectedly.

### Step 4 — Label the Level File

If the Levels panel shows only the filename (e.g., `Level.umap`), rename the asset to reflect the shot context before saving:

```text
Scene_01_Shot_03.umap
```

This naming aligns the level file with the snapshot naming pattern and makes it immediately identifiable in the Content Browser.

### Step 5 — Package as a Level Snapshot (Optional)

For a more structured handoff, use the **Level Snapshot** system:

1. Open the **Window → Level Snapshots** panel.
2. Click **Take Snapshot** to capture the current level state.
3. Give the snapshot a descriptive name matching the `Scene_xx_Shot_xx` convention.
4. Snapshots are stored as separate assets in the Content Browser and can be restored or compared later.

Level Snapshots are particularly useful when you want to preserve a review-ready state without modifying the current working level, or when handing off to a teammate who needs to restore a specific scene without overwriting ongoing work.

### Step 6 — Verify Before Handoff

Run through the handoff checklist before sharing the packaged level. Confirm that all actors are correctly placed, that lighting is in a reviewable state, and that no experimental or unstable actors remain in the scene.

## What Should Be Stable Before Packaging

1. The scene contains the correct actors for the shot context.
2. Essential transforms and placements are no longer exploratory.
3. The lighting state is clear enough to support review.
4. The scene naming and folder structure do not create ambiguity.

## What Packaging Does Not Mean

Packaging does not mean the shot is final. It means the scene state is coherent enough to preserve, review, reuse, or hand to another teammate without confusion.

## Handoff Checklist

- Does the snapshot name follow `Scene_xx_Shot_xx` exactly?
- Is the shot context obvious from the stored scene state?
- Can another teammate restore the snapshot and understand its purpose quickly?
- Are unstable experiments removed or clearly separated before packaging?