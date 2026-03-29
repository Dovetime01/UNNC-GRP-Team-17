---
title: "Unreal Engine Naming Conventions"
description: "Standardized naming practices for assets, actors, and scenes in UE virtual production."
weight: 5
series: ["Scene Packaging Foundations"]
series_order: 1
showDate: false
tags: ["naming", "organization", "asset-management"]
---

## Why Naming Matters

Clear, consistent naming prevents confusion in collaborative VP workflows. When multiple team members work on the same level, assets must be instantly identifiable—by function, type, and context. Poor naming creates bottlenecks: teammates waste time guessing what an asset is, what it does, and why it exists.

## Core Naming Principles

1. **Purpose First**: The name should communicate what the asset does or represents.
2. **Consistency**: Follow the same pattern across the project.
3. **Readability**: Use PascalCase or snake_case, never ambiguous abbreviations.
4. **Scalability**: Naming rules should work whether you have 50 actors or 500.

## Asset Naming Conventions

### Meshes (Static and Skeletal)

**Pattern:** `[Type]_[Description]_[Variant]`

**Examples:**
- `SM_Bed_Hospital_01` — Static mesh, hospital bed, variant 01
- `SM_Door_Wood_Tall` — Static mesh, wooden door, tall variant
- `SK_Talent_Female_02` — Skeletal mesh, female talent character, version 02

**Prefixes:**
- `SM_` → Static Mesh
- `SK_` → Skeletal Mesh

### Materials and Material Instances

**Pattern:** `M_[Surface]_[Property]` or `MI_[Surface]_[Property]_[Variant]`

**Examples:**
- `M_Wall_Concrete` — Material, concrete wall surface
- `MI_Wall_Concrete_Dusty` — Material instance, dusty variant
- `M_Metal_Brushed_Cool` — Material, brushed metal with cool tone

**Prefixes:**
- `M_` → Master Material
- `MI_` → Material Instance

### Textures

**Pattern:** `T_[Surface]_[MapType]` where MapType is `Diffuse`, `Normal`, `Rough`, `Metal`, `Emissive`

**Examples:**
- `T_Brick_Diffuse` — Diffuse/color texture for brick
- `T_Wood_Normal` — Normal map for wood grain
- `T_Metal_Rough` — Roughness map for metal

### Blueprints and Actors

**Pattern:** `BP_[FunctionType]_[Description]` or `AC_[Type]_[Description]`

**Examples:**
- `BP_Light_KeyLight` — Blueprint, key light actor
- `BP_Interactive_Door` — Blueprint, interactive door with logic
- `AC_Camera_Cinematic` — Actor, cinematic camera setup

**Prefixes:**
- `BP_` → Blueprint Class
- `AC_` → Actor (non-blueprint)

### Levels and Scenes

**Pattern:** `Level_[Scene]_[Purpose]` or `Scene_[Number]_Shot_[Number]`

**Examples:**
- `Level_HospitalRoom_Master` — Master/base level for hospital room
- `Level_Street_Exterior_Night` — Street exterior, night setup
- `Scene_01_Shot_01` — Scene 1, Shot 1 (for previs snapshots)

## Light Naming

**Pattern:** `Light_[FunctionType]_[Description]`

**Examples:**
- `Light_Key_Hard` — Key light, hard quality
- `Light_Fill_Warm` — Fill light, warm color tone
- `Light_Backlight_Practical` — Backlight simulating practical source
- `Light_Sky_Ambient` — Sky light for ambient illumination

**Common Function Types:**
- `Key` — Primary light source
- `Fill` — Secondary light for shadow fill
- `Backlight` — Separation/rim light
- `Accent` — Localized effect light
- `Ambient` / `Sky` — Global illumination
- `Practical` — Light embedded in scene props

## Variable and Component Naming

### Naming Blueprint Variables

**Pattern:** Use descriptive PascalCase with type inference

**Examples:**
- `bIsLocked` — Boolean variable, "is locked" state
- `LightIntensity` — Float variable, light intensity value
- `TargetLocation` — Vector variable, target position
- `ActorReferences` — Array of actor references

**Naming Conventions:**
- Booleans: Start with `b` followed by PascalCase (`bIsActive`, `bCanMove`)
- Numbers (int/float): Plain PascalCase (`Speed`, `Health`, `Intensity`)
- References: Descriptive PascalCase (`PlayerCharacter`, `TargetActor`)
- Arrays: Plural or suffix with `Array` (`Lights`, `ActorsToSpawn`, `LightArray`)

### Naming Components

**Pattern:** `[ComponentType]_[Description]`

**Examples:**
- `StaticMesh_Floor` — Static mesh component for floor
- `PointLight_Lamp` — Point light component for lamp
- `Camera_Main` — Camera component, main view

## Folder Structure Alignment

Keep folder organization mirrors naming conventions:

```
Content/
├── Assets/
│   ├── Meshes/
│   │   ├── StaticMeshes/
│   │   │   └── SM_*.uasset
│   │   └── SkeletalMeshes/
│   │       └── SK_*.uasset
│   ├── Materials/
│   │   ├── M_*.uasset
│   │   └── Instances/
│   │       └── MI_*.uasset
│   └── Textures/
│       └── T_*.uasset
├── Blueprints/
│   ├── BP_*.uasset
│   └── Actors/
│       └── BP_*_*.uasset
└── Levels/
    ├── Level_*.uasset
    └── Scene_*_Shot_*.uasset
```

## Conclusion

Consistent naming is a small investment with large returns. It accelerates onboarding, reduces miscommunication, and makes scene management scalable. Start with this convention and adapt it to your team's specific needs—but keep the core principle: **the name should answer "what is this?" without requiring additional context**.

For more in-depth guidance, refer to the [official UE naming conventions documentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/recommended-asset-naming-conventions-in-unreal-engine-projects).
