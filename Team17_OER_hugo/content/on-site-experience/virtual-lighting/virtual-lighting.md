---
title: "Lighting Architecture and On-Set Implementation"
description: "Understanding and implementing lighting systems in Unreal Engine for virtual production."
date: 2025-03-26T10:00:00+08:00
weight: 10
tags: ["lighting", "light-actors", "snapshots", "on-set-lighting"]
---

## Introduction

Lighting in virtual production serves dual roles: it creates the visual atmosphere the camera sees, and it provides the technical foundation for real-time adaptation during shooting. This page covers the practical implementation of lighting in Unreal Engine, from understanding light actor types to managing lighting state across multiple shots using Level Snapshots.

## 1. Light Actors in Unreal Engine

Unreal Engine provides multiple light actor types, each optimized for specific roles in scene lighting.

### 1.1 Directional Light

**Physical Analog:** The sun or a distant light source that casts parallel light rays.

**Characteristics:**
- Casts parallel light rays across the entire scene
- Direction matters; intensity of rays is consistent regardless of distance
- One primary light source typically dominates a scene (the key light or sun)
- Relatively low computational cost

**VP Use Cases:**
- Primary key light (simulating main production lights or sunlight)
- Fill light direction (secondary parallel light source)
- Backlight in traditional three-point setups

**Properties to Control:**
- **Intensity**: Measured in Lumens (for directional lights, this represents total light output)
- **Light Color**: RGB value, often adjusted by Color Temperature (in Kelvin)
- **Rotation**: Direction the light points (affects shadow direction)

**Example Workflow:**
```
1. Select Directional Light in Outliner (e.g., "Key Light")
2. Modify Rotation to adjust shadow direction (aim light vector forward)
3. Adjust Intensity (e.g., 50000 Lumens for bright key light)
4. Adjust Light Color or use Temperature (e.g., 5600K for daylight, 3200K for tungsten)
```

### 1.2 Point Light

**Physical Analog:** A light bulb, candle, or other omnidirectional point source.

**Characteristics:**
- Emits light in all directions from a single point
- Intensity falls off with distance
- Useful for localized scene illumination (lamps, sconces, practical lights)
- Moderately higher computational cost then directional lights

**VP Use Cases:**
- Practical lights (table lamps, ceiling fixtures, candles)
- Fill lights positioned near actors or set pieces
- Accent lights highlighting specific objects
- Emissive surfaces that cast dynamic light (screens, neon signs)

**Properties to Control:**
- **Intensity**: Measured in Lumens
- **Light Color**: RGB or Temperature-based
- **Location**: Position in 3D space
- **Attenuation Radius**: How far light spreads before cutoff

**Example Workflow:**
```
1. Place Point Light actor in scene (Add Actor → Light → Point Light)
2. Position near intended affect area (e.g., beside a desk lamp)
3. Set Intensity (e.g., 1500-3000 Lumens for a typical table lamp)
4. Adjust Attenuation Radius to control light falloff range
5. Enable/disable based on shot requirements
```

### 1.3 Spot Light

**Physical Analog:** A flashlight, stage spotlight, or focused light source.

**Characteristics:**
- Emits light in a cone-shaped pattern
- Can create sharp falloff and controlled illumination
- Highly directional; useful for precise lighting control
- Computational cost between Point and Directional lights

**VP Use Cases:**
- Stage lighting effects (theater or studio setups)
- Focused accent lights on talent or specific set pieces
- Practical lights with directional control (desk lamps with shades)
- Dramatic lighting effects

**Properties to Control:**
- **Intensity**: Measured in Lumens
- **Light Color**: RGB or Temperature-based
- **Location and Rotation**: Position and direction of cone
- **Outer Cone Angle**: Width of the light beam
- **Inner Cone Angle**: Radius of the sharp falloff area
- **Attenuation Radius**: Maximum light reach

**Example Workflow:**
```
1. Place Spot Light actor in scene
2. Position and rotate to direct light toward target area
3. Adjust Outer Cone Angle (e.g., 45 degrees for focused effect)
4. Set Inner Cone Angle for soft vs. hard falloff
5. Adjust Intensity and Attenuation Radius
```

### 1.4 Sky Light

**Physical Analog:** Ambient sky illumination from the sun and atmosphere.

**Characteristics:**
- Provides indirect diffuse illumination (simulating light bounced from the sky)
- Often sampled from an HDRI cubemap or simplified sky atmosphere
- Affects the overall ambient lighting tone
- Relatively low computational cost if using static capture

**VP Use Cases:**
- Provide base ambient illumination for interior or exterior scenes
- Control overall scene brightness and color temperature
- Fill light that simulates skylight bounce in real interiors
- Establish visual coherence across the entire environment

**Properties to Control:**
- **Intensity**: Multiplier on the sky's contribution
- **Sky Cubemap**: HDRI image or procedural sky source
- **Lower Hemisphere Color**: Specific color for light bouncing from ground

**Example Workflow:**
```
1. Place Sky Light in scene (usually one per level)
2. Set Sky Cubemap to an HDRI match the scene environment
3. Adjust Intensity (e.g., 1.0 for realistic indirect light)
4. Fine-tune Lower Hemisphere Color if needed for warm/cool fill
```

## 2. Lighting Measurement Systems

Understanding how UE measures light is essential for consistent lighting and communication with physical production teams.

### 2.1 Lumens (lm)

**Definition:** Total light output from a source, measured in luminous flux.

**Physical Reference:**
- 100W incandescent bulb ≈ 1,600 lm
- 1000W tungsten light on set ≈ 25,000 lm
- Direct sunlight ≈ 100,000+ lm

**UE Application:**
- Primary unit for Directional, Point, and Spot light intensity
- Scale relative to real-world lighting setups for physical accuracy
- Higher values = brighter lights

**Practical VP Usage:**
```
Directional Light (Sun equivalent):     50,000+ Lumens
Directional Light (Studio key):         20,000 - 40,000 Lumens
Point Light (2000W practical):          5,000 - 8,000 Lumens
Point Light (table lamp):               500 - 2,000 Lumens
Sky Light intensity:                    0.8 - 2.0 (multiplier on cubemap)
```

### 2.2 Candela (cd)

**Definition:** Luminous intensity (light per solid angle), used for Spot Lights.

**Relationship to Lumens:**
- For Spot Lights, intensity may be expressed as Candela
- Conversion depends on cone angle; narrower cones concentrate lumens into higher candela

**UE Application:**
- Spot lights can be measured in Lumens (total output) or effective candela
- Understand cone angle when comparing intensity values

### 2.3 EV (Exposure Value) and Stops

**Definition:** Logarithmic scale matching photography exposure, useful for relative adjustments.

**VP Application:**
- One EV or "stop" = 2x change in brightness
- Useful for quick, intuitive adjustments ("add one stop of fill light")
- Easier communication with cinematographers accustomed to photography terms

**Conversion Reference:**
- +1 EV = 2x brightness (multiply intensity by 2)
- -1 EV = 0.5x brightness (multiply intensity by 0.5)
- Useful for incremental on-set adjustments

**Example:**
```
Key Light at 30,000 Lumens is too bright.
Reduce by 1 stop: 30,000 ÷ 2 = 15,000 Lumens
Still too bright? Reduce by another 0.5 stops: 15,000 ÷ 1.414 ≈ 10,600 Lumens
```

### 2.4 Color Temperature (Kelvin)

**Definition:** Color description of light, from cool (blue, high K) to warm (orange, low K).

**Practical Reference:**
- 2000K: Candlelight (very warm, orange)
- 3200K: Tungsten bulb (warm, traditional studio light)
- 5600K: Daylight (neutral, standard daylight)
- 6500K: Daylight (slightly cool, overcast sky)
- 10000K+: Overcast/shaded sky (very cool, blue)

**UE Application:**
- Adjust Light Color directly (RGB values), or
- Use Color Temperature picker for intuitive warm/cool adjustment
- Critical for matching practical lights to daylight or vice versa

## 3. Establishing Lighting Baselines

A lighting baseline is the starting point for a scene—the core configuration that defines the visual tone and provides a foundation for shot-specific modifications.

### 3.1 Baseline Lighting Approach

**Step 1: Define the Light Source Type**

Determine what physical light source the scene uses:
- Sunlit exterior (Directional Light as key)
- Interior practical lights (Point/Spot Lights as key)
- Mixed lighting (practical lights + fill bounce)
- Studio setup (hard key + reflector fill)

**Step 2: Establish Key Light**

Position and configure the primary light source:
- Direction: Appropriate angle for intended mood (front-lit, side-lit, back-lit)
- Intensity: Bright enough to illuminate the subject without washing out detail
- Color: Matched to the established time of day or practical light type

**Example: Interior Scene with Practical Key**
```
Light: "Desk Lamp Key"
Type: Point Light
Position: On desk left side, 2 units above desktop
Intensity: 3000 Lumens
Color: 3200K (tungsten bulb)
Attenuation Radius: 800 units (enough to light the desk and nearby areas)
```

**Step 3: Add Fill Light**

Secondary light source to reduce shadow harshness:
- Usually 25-50% intensity of key light
- Often positioned opposite the key to brighten shadows
- Can be Directional (for large area fill) or Point (for localized bounce)

**Example: Interior Scene Fill**
```
Light: "Room Fill"
Type: Directional Light or Sky Light
Intensity: 8000 Lumens (25% of 30,000 lumen key)
Color: Slightly warmer than key (soften shadows)
Role: Provide base illumination so shadows aren't pure black
```

**Step 4: Add Backlight (Optional)**

Creates separation between subject and background:
- 50-100% intensity of key light
- Often behind and above the subject
- Creates rim/edge light that separates foreground from background

**Example: Interior Scene with Backlight**
```
Light: "Backlight"
Type: Directional Light or Spot Light
Intensity: 25000 Lumens (can match key light for dramatic effect)
Color: Slightly warmer or cooler than key for contrast
Direction: Behind camera view, above subject eye line
```

### 3.2 Documenting Baseline State

Create a reference document for each scene's baseline lighting:

```
Scene: "Hospital Room - Night"
Primary Source: Bedside lamp + ceiling cove light

Key Lights:
- "Bedside_Key": Point Light, 2500 Lumens, 3200K, Radius 600
- "Ceiling_Fill": Directional Light, 5000 Lumens, 3400K

Supporting Lights:
- "Window_External": Directional Light, 15000 Lumens, 5600K, Angle: -30 degrees
- "Monitor_Glow": Point Light, 1200 Lumens, 5000K (emissive screen effect)

Baseline Mood: Dim, warm interior with cool backlight from window
Typical Shadow Intensity: Visible but not black (shadows retain detail)
```

## 4. Level Snapshots for Multi-Shot Lighting Management

Level Snapshots are UE's tool for capturing and restoring actor state. For lighting-heavy scenes with multiple shots, snapshots enable:
- Rapid switching between different lighting configurations
- Encapsulation of lighting state "per shot"
- Non-destructive comparison between setups
- Efficient rollback if on-set changes need reverting

### 4.1 What Level Snapshots Capture

A snapshot records the state of actors in a level, including:
- Actor position, rotation, scale
- Light intensity, color, attenuation
- Material properties
- Actor visibility
- Component parameters

**Not Captured:**
- Asset references (if you change the mesh/material type, snapshot doesn't revert that)
- Post-process settings in most cases
- Camera settings (captured separately)

### 4.2 Creating a Snapshot Per Scene

**Workflow: Set Up Baseline, Then Create Snapshots**

```
1. Build and light your scene (baseline configuration)
   - Arrange all furniture
   - Configure all lights
   - Verify look across multiple camera angles

2. Create Snapshot for Baseline:
   → Tools → Create New Snapshot
   → Name: "Scene_01_Shot_01_Baseline"
   → Save

3. Modify lights/actors for Shot 02:
   - Adjust key light color to 5600K for cool tone
   - Move backlight 200 units to stage right
   - Dim fill light by 2000 Lumens
   → Tools → Create New Snapshot
   → Name: "Scene_01_Shot_02_CoolTone"
   → Save

4. During on-set production:
   - Quick compare between snapshots by loading each one
   - If a shot needs the baseline look again, load "Scene_01_Shot_01_Baseline"
   - If modification breaks continuity, load previous snapshot and try different adjustment
```

### 4.3 Snapshot Naming Convention

Adopt a consistent naming scheme to quickly identify snapshots:

```
[Scene]_[Shot]_[Descriptor]

Examples:
- Scene_01_Shot_01_Baseline
- Scene_01_Shot_02_BacklightRight
- Scene_01_Shot_05_DayExterior_KeyPlus2Stops
- Scene_02_Shot_08_NightAmbience
```

Benefits:
- Quickly identify which snapshot applies to which shot
- Immediately understand the modification  (e.g., "Backlight Right")
- Sort chronologically or by scene

### 4.4 Practical Snapshot Workflow During Production

**Scenario: Multiple Shots, Same Location**

```
Scene: Hospital room with three camera angles (Wide, Medium, Close-up)

Preparation:
1. Create baseline lighting (all three shots use same key light position/intensity)
2. Create "Scene_01_BaselineLighting"
3. Adjust fill light color for slightly warmer tone (production preference)
4. Create "Scene_01_WarmFill" (difference: +500K color temp on fill)
5. Prepare backlight adjustment (if wide shot needs back fill)
6. Create "Scene_01_WithBackFill"

Production:
Director: "Give me one more shot with the backlight a bit softer."
TD: Load "Scene_01_WithBackFill" → Reduce backlight intensity from 20000 to 15000 Lumens
    → Verify in playback → Save as "Scene_01_WithBackFill_Softer"

Director: "Go back to baseline, but keep that warmer fill."
TD: Load "Scene_01_WarmFill"
```

## 5. Common On-Set Lighting Modifications

Real-time lighting adjustments during production usually fall into a few categories:

### 5.1 Intensity Adjustments (Brightness)

**Common Request:** "The shadow is too dark" or "Backlight is overpowering the face"

**Solution:**
1. Identify which light to modify (key, fill, or backlight)
2. Reduce or increase intensity in 5-10% increments
3. Compare with reference footage if available
4. Use EV/stops terminology for communication with DOP

**Implementation:**
```
Select light in Outliner → Details Panel → Lights → Intensity
Modify value and observe real-time viewport feedback
Record new snapshot if change becomes permanent
```

### 5.2 Color/Temperature Adjustments

**Common Request:** "The room looks too cool" or "Add more warmth to the shadow"

**Solution:**
1. Adjust Color Temperature slider (warmer = lower K, cooler = higher K)
2. Or manually modify RGB color in Details
3. Compare against practical light sources in the scene
4. Verify continuity with other shots using the same color lights

**Implementation:**
```
Select light → Details → Light Color
Use Temperature slider (3000K warm → 4000K neutral → 5600K cool)
Or use RGB picker and adjust toward yellow/orange (warm) or blue (cool)
```

### 5.3 Position/Direction Adjustments

**Common Request:** "Move the key light more to camera left" or "Lower the backlight angle"

**Solution:**
1. Select light actor in viewport or Outliner
2. Use transform gizmo (Move/Rotate) or Details Panel coordinates
3. Make incremental adjustments (5-10 units at a time)
4. Check for shadow direction changes and light spill
5. Verify continuity if light affects multiple shots

**Implementation:**
```
Select light → Press W (Move) or R (Rotate) key
Drag gizmo or use Details Panel to adjust rotation
For Directional lights: rotation defines light direction
For Point lights: location defines light position
```

### 5.4 Attenuation/Falloff Adjustments

**Common Request:** "The point light is spilling too much into the background" or "Tighten the spotlight beam"

**Solution:**
1. For Point/Spot lights, adjust Attenuation Radius to control how far light spreads
2. For Spot lights, adjust Outer Cone Angle to widen/narrow the beam
3. Small adjustments create noticeable changes in falloff

**Implementation:**
```
Point Light specifics:
- Select light → Details → Light → Attenuation Radius
- Smaller radius = light only affects nearby objects
- Larger radius = light spreads farther

Spot Light specifics:
- Select light → Details → Light → Outer Cone Angle
- Smaller angle = tighter beam (more focused)
- Larger angle = wider beam (more spread)
```

## 6. Practical Tips for Real-Time Lighting on Set

### 6.1 Organize Lights in the Outliner

Use naming and hierarchy to keep lights quickly accessible:

```
Lights (Folder)
├── Key_Lights
│   ├── Key_Light_Primary
│   └── Key_Light_Backfill
├── Fill_Lights
│   └── Room_Fill
├── Accent_Lights
│   ├── Lamp_Desk
│   ├── Lamp_Ceiling
│   └── Window_Glow
└── Sky_Light
    └── Ambient_Sky
```

**Benefit:** Can quickly locate and select the light you need to adjust

### 6.2 Create Light Sets by Shot

If scenes have dramatically different lighting per shot, group snapshots by shot:

```
In production folder structure:
/Snapshots/
  /Scene_01/
    /Shot_01_Wide/
      Scene_01_Shot_01_Baseline
      Scene_01_Shot_01_Alt_MoreBacklight
      Scene_01_Shot_01_Alt_WarmFill
    /Shot_02_Medium/
      Scene_01_Shot_02_Baseline
      ...
```

### 6.3 Use Real-Time Playback to Evaluate Changes

Don't rely solely on static viewport. Play back recorded footage or camera movement to see lighting in motion:
- Playback shows how light changes with camera movement
- Motion reveals shadow quality and light spill issues
- More accurate for evaluating director intent

**Workflow:**
```
Make lighting adjustment → Play in Editor (Alt+P) or start Cinematic Playback
Review lighting quality through camera movement
Pause, tweak, play again
```

### 6.4 Communicate in Stopping and EV Terms

When DOP or director asks for adjustments, clarify using standardized terminology:

**Standard Communication:**
- "Reduce key light by half a stop" = multiply intensity by 0.707
- "Add one stop of fill" = multiply fill intensity by 2
- "Pull back the backlight" = reduce intensity or attenuation radius
- "Warm up the fill" = shift Color Temperature toward orange (lower K)

## Conclusion

Lighting in virtual production is a balance between technical precision and creative flexibility. Understanding light actor types, measurement systems, and snapshot-based organization enables rapid implementation of complex lighting during production. Start with solid baselines, document state through snapshots, and embrace incremental adjustments. This foundation allows the technical and creative teams to iterate efficiently without losing visual continuity or technical coherence.
