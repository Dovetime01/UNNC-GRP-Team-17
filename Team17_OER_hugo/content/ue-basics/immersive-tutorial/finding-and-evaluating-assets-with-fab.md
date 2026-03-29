---
title: "Finding and Evaluating Assets with Fab"
description: "Learn how to effectively search, evaluate, and acquire assets for virtual production using Unreal Engine's Fab marketplace."
weight: 25
series: ["Immersive Tutorial Foundations"]
series_order: 3
categories: ["Immersive Tutorial"]
tags: ["fab", "assets", "marketplace"]
showDate: false
---

## What This Solves

In virtual production, building environments from scratch is often too slow. Teams rely on marketplaces to accelerate scene assembly. Fab is Epic Games' unified content marketplace, integrating assets from the Unreal Engine Marketplace, Quixel Megascans, Sketchfab, and ArtStation. This guide covers how to efficiently find and evaluate assets before bringing them into your previs scene.

## 1. Finding Assets in Fab

Fab can be accessed directly within Unreal Engine or via a web browser. Using it inside the engine is typically the fastest way to drop assets into an active scene.

### Searching Effectively
- **Use Specific Keywords:** Instead of "chair," search for "hospital chair" or "mid-century wooden chair."
- **Filter by Category:** Narrow down your search to 3D Models, Materials, or Environments to avoid clutter.
- **Filter by License:** Ensure you are looking at assets that match your project's budget and licensing requirements (e.g., Free, Standard).

## 2. Evaluating Assets Before Downloading

Not all assets are optimized for virtual production. Before downloading, evaluate the asset against these criteria:

### Visual Quality
- **Art Style:** Does the style match your scene? (e.g., photorealistic vs. stylized)
- **Texture Resolution:** Are the textures high enough resolution for your camera's intended proximity?

### Technical Suitability
- **Poly Count (Triangle Count):** Is the asset too heavy for real-time performance? Extremely high-poly assets might require Nanite enabled to run smoothly.
- **Material Complexity:** Does it use complex or custom shaders that might affect your existing scene lighting performance?
- **Scale Accuracy:** Is it built to real-world scale (1 Unreal Unit = 1 cm)?

## 3. Bringing Assets into Unreal Engine

Once you have evaluated and selected an asset:

1. Open the **Fab plugin/window** inside Unreal Engine.
2. Click **Add to Project** or drag and drop the asset directly into your Content Browser.
3. Place the asset in your Level Viewport.
4. **Immediate Validation:** Always check the newly imported asset against a reference character (like a standard UE mannequin) to ensure the scale is correct. Verify that its materials respond correctly to your established baseline lighting.

## Further Reading

For a deep dive into managing licenses, plugins, and advanced Fab features, refer to the official [Fab Documentation](https://dev.epicgames.com/documentation/en-us/fab/fab-documentation).