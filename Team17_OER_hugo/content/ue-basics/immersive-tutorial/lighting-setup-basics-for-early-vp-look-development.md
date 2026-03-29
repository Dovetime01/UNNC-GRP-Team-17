---
title: "Lighting Setup Basics for Early VP Look Development"
description: "Establish a reliable lighting baseline for early virtual production look development and previs reviews."
weight: 35
series: ["Immersive Tutorial Foundations"]
series_order: 4
categories: ["Immersive Tutorial"]
tags: ["lighting", "look-development", "previs", "virtual-production"]
showDate: false
---

## What This Solves

In early virtual production look development, teams often make visual decisions before the lighting baseline is stable. That leads to inconsistent reviews: one version may look cinematic, while the next looks flat only because exposure, sky intensity, or key/fill balance changed. This guide provides a simple lighting setup workflow so early scene reviews are comparable and actionable.

## 1. Start with a Neutral Baseline

Before shaping mood, create a clean and readable default state:

1. Add or verify a **Directional Light** as your main source.
2. Add a **Sky Light** for ambient fill.
3. Add a **Sky Atmosphere** and, if needed, **Exponential Height Fog** for depth cues.
4. Lock your camera exposure approach (manual or controlled auto exposure) before judging contrast.

A neutral baseline is not your final look. It is a controlled starting point that helps your team evaluate composition, scale, and material response without misleading lighting changes.

## 2. Build a Simple Key-Fill-Separation Structure

Use a practical three-part structure for early look development:

- **Key Light:** Defines primary direction and shadow shape.
- **Fill Light / Ambient Fill:** Lifts shadows enough to preserve important detail.
- **Separation (Rim/Back) Light:** Helps subjects read against the background.

For previs, keep this structure simple and readable first. Fine cinematic nuance can be layered later once blocking and scene intent are stable.

## 3. Keep Lighting Decisions Review-Friendly

When preparing a scene for internal review:

- Keep light naming clear (for example, `Light_Key_Main`, `Light_Fill_Soft`, `Light_Back_Separation`).
- Avoid stacking many experimental lights with overlapping purpose.
- Verify that hero assets still read correctly from your primary camera angles.
- Check that material highlights and shadows remain coherent after any exposure changes.

If reviewers cannot tell whether a change came from layout or lighting, iteration quality drops. Review-ready lighting should make scene intent obvious.

## 4. Common Early-Stage Pitfalls

- **Over-bright Sky Light:** Reduces depth and makes everything look uniformly flat.
- **Uncontrolled Exposure:** Causes false comparisons between versions.
- **Too Many Local Accent Lights Too Early:** Adds visual noise before core look decisions are made.
- **Ignoring Scale Cues:** Lighting may look correct in isolation but fail when viewed with characters or known-size props.

Treat these as troubleshooting checkpoints whenever a scene suddenly becomes hard to read.

## Quick Validation Checklist

Before handing the scene to teammates or review:

- Is the main light direction intentional and easy to read?
- Are shadows detailed enough without crushing key information?
- Is exposure behavior stable across the primary review cameras?
- Do subjects separate clearly from the background?
- Can another teammate understand light purpose from names alone?

## Further Reading

For detailed controls and advanced workflows, refer to Unreal Engine documentation on [Lighting the Environment](https://dev.epicgames.com/documentation/en-us/unreal-engine/lighting-the-environment-in-unreal-engine) and [Post Process Effects](https://dev.epicgames.com/documentation/en-us/unreal-engine/post-process-effects-in-unreal-engine).
