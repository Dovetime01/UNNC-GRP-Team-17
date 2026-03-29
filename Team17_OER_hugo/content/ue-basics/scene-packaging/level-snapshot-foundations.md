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