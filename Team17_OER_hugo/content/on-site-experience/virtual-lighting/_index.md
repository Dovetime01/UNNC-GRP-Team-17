---
title: "Virtual Lighting"
description: "Lighting systems, actors, and frameworks for real-time virtual production."
weight: 20
showDate: false
showTaxonomies: false
---

{{< lead >}}
Lighting in virtual production is not simply about illumination—it is a technical and creative system that must respond in real-time to director intent, adapt across multiple shots, and maintain consistency across different camera angles and takes. This section introduces UE lighting architecture, the actors and tools that control light behavior, and frameworks for organizing lighting state across complex, multi-shot environments.
{{< /lead >}}

## Learning Outcome

By the end of this section, learners should be able to identify and use different light actor types in Unreal Engine, understand lighting measurement systems and their practical applications, establish consistent lighting rigs that support multiple shots, and manage lighting state efficiently using Level Snapshots.

## This Section Covers

1. Light actor types and their roles in VP lighting
2. Lighting measurement systems (Lumens, Candela, EV, etc.) and practical calibration
3. Lighting rigs and scene-based organization strategies
4. Level Snapshots as a tool for multi-shot lighting management
5. Common on-set lighting adjustments and their impact

## Key Concepts

- **Light Actors**: Directional, Point, Spot, and Sky lights, each with specific use cases
- **Intensity Units**: Understanding how UE measures light and translating to physical setups
- **Lighting Baselines**: Establishing a consistent baseline lighting configuration for a scene
- **Snapshot Architecture**: Using Level Snapshots to encapsulate lighting state across multiple shots
- **Real-Time Adaptation**: Modifying lights during production without breaking continuity

## Why This Matters

Virtual production compresses the lighting design process. Traditional film sets have weeks to dial in lighting. VP sets have minutes. Understanding how to establish a coherent lighting system, document it through snapshots, and adapt it to real-time production needs directly impacts the efficiency and visual quality of the shoot. Lighting is also where technical and creative decisions intersect most directly—a strong lighting framework enables directors to explore creative choices confidently.
