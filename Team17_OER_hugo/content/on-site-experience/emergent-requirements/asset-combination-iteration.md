---
title: "Asset Combination and Iteration — Building the Window Model"
description: "How the team assembled a window prop for a period interior by combining a reference photograph with existing 3D assets, and the technical iteration that followed when the initial result failed real-time performance requirements."
weight: 12
series: ["On-Site Asset Production"]
series_order: 2
categories: ["On-Site Experience", "Asset Production"]
tags: ["asset-pipeline", "iteration", "window-model", "iron-bars", "performance", "ue-technical-lead"]
showDate: false
---

## Context

Not every prop is generated or modeled from scratch. Sometimes the fastest path to a usable asset runs through **combination** — taking an existing 3D model, adapting it with reference imagery, and modifying it to match a specific look. The window prop used in this project illustrates this process from brief to final usable asset.

The director wanted a window with two distinct visual elements: iron bars across the opening and a white wooden cross-frame inside the bars. The bar style had a specific look — period-accurate, solid, black iron — and the wooden frame had a painted white finish with a cross-mullion pattern. Both needed to appear together and read correctly on camera.

---

## The Assembly Process

### Step 1 — Creating the iron railing model

The art direction began with a reference image found online that matched the director's description for the iron bars: solid, dark, period-appropriate in character. Therefore, we uploaded this image to [Tripo](/vp-workflow/ai-asset-generation/) to generate an initial 3D model.

![Iron window frame](/asset-combination/iron-window.jpg)

---
### Step 2 — Sourcing base assets to build the window frames

A window frame model was already available from an open-source library. This frame had the right general proportions and was structurally complete enough to serve as a base. It needed significant modification — specifically the addition of a cross-mullion pattern — but the basic geometry was sound.

### Step 3 — Add the Cross-Mullion Pattern

The wooden cross-frame was constructed manually. Additional geometry was added to the existing frame mesh to create the central horizontal and vertical dividers that form the cross pattern. The mesh was cleaned and optimized after this addition to remove any overlapping geometry.

### Step 4 — Combine Iron Bars with Frame

The iron bar reference informed the placement of a separate bar element over the window opening. The bars were positioned to sit cleanly in front of the wooden frame, and material assignments were made: dark metallic for the bars, flat white painted wood for the frame.

The assembled window matched the director's reference closely enough to pass an initial review.

![Assembled window model — iron bars combined with cross-mullion wooden frame](/asset-combination/window-assembled1.jpg)

---

## A Performance Problem Emerges

However, during the test shooting in the VP Studio, a new problem appeared. When the camera moved through the scene during playback, the iron bar geometry produced visible **flickering** — a strobing artifact caused by Z-fighting between overlapping surfaces in the rendering pipeline. This made the window unusable for production.

![Z-fighting artifact on iron bars during camera movement](/asset-combination/window-flickering.jpg)

The cause was high polygon density in the iron bar mesh. When rendered in real-time against the scene background, the overlapping geometry at slightly different depths confused the depth buffer, producing the flicker artifact. This is a common failure mode with high-density meshes in VP environments where camera movement exposes rendering edge cases that a static render would not.

---

## The Resolution

The iron bar mesh was reworked by the UE technical lead. We adopted a new model with a lower polygon count.

The revised mesh was re-imported and verified. The flicker was resolved and the window passed final review.

![Final window model after polygon optimization — flicker resolved](/asset-combination/window-final1.jpg)
![Final window model after polygon optimization — flicker resolved](/asset-combination/window-final2.jpg)

---

## What This Workflow Taught Us

### Combination Is a Valid Production Strategy

Starting from an existing open-source asset, adapting it with a reference image, and manually adding missing geometry is not a shortcut — it is a **legitimate production workflow**. It is faster than modeling from scratch and produces more reliable results than AI generation when the look needs to match a specific reference exactly.

### Reference Imagery Has a Specific Role

The iron bar photograph was not a texture. It was a **visual specification** that guided geometry decisions. Knowing how to extract design intent from a reference — rather than treating it as a direct source — is a skill that separates effective asset assembly from literal copying.

### Iteration Is Part of the Pipeline, Not an Exception

The flicker problem was not a failure. It was a normal iteration step that revealed a technical constraint invisible at the asset creation stage. Real-time rendering exposes issues that static rendering does not: depth conflicts, performance spikes, LOD failures. Building time for iteration into the asset pipeline is not optional — it is how production-quality results are actually achieved.

### UE Technical Expertise Was Essential

The resolution required someone who understood both the artistic intent of the window and the real-time rendering behavior causing the flicker. This is why close collaboration between the art team and UE technical leads matters — especially for props that appear in camera-facing shots where rendering artifacts are visible.

---

## Takeaway

Asset combination workflows work well when:
- A reference or existing asset exists that can be adapted
- The fidelity bar is known in advance (period accuracy, close-up vs. background)
- Time exists for at least one iteration pass after initial integration

The window model succeeded because the team had the skills to assemble it, the reference to guide the look, and the technical capacity to iterate when the real-time environment revealed a problem. Each of those three elements was necessary; none alone was sufficient.
