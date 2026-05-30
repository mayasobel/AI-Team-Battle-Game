# ⚔️ AI Team Battle Simulation

A real-time 2D combat simulation built in **C++ with OpenGL (FreeGLUT)**, where two AI-controlled teams fight autonomously in a procedurally generated dungeon. Each team consists of **Fighters**, a **Medic**, and a **Supplier** — all driven by individual Finite State Machines (FSM) with unique randomized personalities.

![Language](https://img.shields.io/badge/Language-C++-blue?logo=cplusplus)
![Graphics](https://img.shields.io/badge/Graphics-OpenGL%20%2F%20FreeGLUT-green)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?logo=windows)
![IDE](https://img.shields.io/badge/IDE-Visual%20Studio-purple?logo=visualstudio)

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [AI System](#ai-system)
- [Dungeon Generation](#dungeon-generation)
- [Class Reference](#class-reference)
- [Build & Run](#build--run)
- [Controls](#controls)
- [Configuration](#configuration)

---

## Overview

The simulation places two teams (🔵 **Blue** and 🔴 **Red**) on opposite sides of a procedurally generated dungeon. Agents navigate rooms and corridors using **A\* pathfinding**, engage in combat with bullets and grenades, request healing and ammo resupply, and make tactical decisions based on their randomized personality traits.

The game ends when:
- One team is fully eliminated → the surviving team wins
- Both teams are eliminated simultaneously → **Draw**
- All ammo is depleted across both teams → **Game Over**

---

## Features

| Feature | Description |
|---|---|
| **Procedural Dungeon** | 8 rooms with random sizes/positions, connected via A\*-carved corridors with obstacle scatter |
| **Finite State Machines** | Each agent type has its own FSM with distinct states and transition logic |
| **Personality System** | Agents spawn with random aggression, risk aversion, and resource thresholds |
| **A\* Pathfinding** | Dual-layer A\* — room-level navigation and cell-level grid movement |
| **Safety Map** | Heat map tracking combat risk per cell/room, influencing pathfinding cost and tactical decisions |
| **Combat System** | Bullets (single-target, line-of-sight) and grenades (area-of-effect) with cooldowns |
| **Support Agents** | Medic heals wounded fighters; Supplier resupplies ammo — both restock at depots |
| **Visual Feedback** | Health/ammo bars, bullet/grenade animations, combat-zone floor darkening, team HUD |
| **Dynamic Risk** | Rooms with active combat become dangerous zones; risk decays over time |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                          main.cpp                               │
│  Game loop · Dungeon generation · Rendering · Team setup        │
└────────────┬──────────────────────────────────┬─────────────────┘
             │                                  │
     ┌───────▼───────┐                 ┌────────▼────────┐
     │   Team (x2)   │                 │   Room (x8)     │
     │ fighters[]    │                 │ depot type      │
     │ medic         │                 │ obstacles       │
     │ supplier      │                 │ connections     │
     └──┬────┬────┬──┘                 └─────────────────┘
        │    │    │
   ┌────▼┐ ┌▼───┐ ┌▼───────┐
   │Fight│ │Med │ │Supplier │      ◄── All inherit from BaseAgent
   │ er  │ │ ic │ │         │
   └──┬──┘ └─┬──┘ └──┬─────┘
      │      │       │
      ▼      ▼       ▼
   ┌──────────────────────┐     ┌───────────────┐
   │    State (FSM)       │────►│  SafetyMap     │
   │ FighterStates (x6)   │     │ cellRisk[][]   │
   │ SupportStates (x5)   │     │ roomRisk[]     │
   └──────────────────────┘     └───────────────┘
```

---

## AI System

### Fighter FSM

Fighters are the combat units. Their state transitions are priority-based:

```
                    ┌───────────────────────┐
                    │      SeekEnemy        │
                    │  (navigate to enemy)  │
                    └───┬──────┬──────┬─────┘
          enemy found   │      │      │  low HP / low ammo
          in room       │      │      │
              ┌─────────▼┐     │   ┌──▼──────────┐
              │  Engage  │     │   │ MoveToMedic  │
              │  Combat  │     │   │ MoveToSupply │
              └──┬───┬───┘     │   └──┬───────┬───┘
        no enemy │   │ out     │      │ close │ healed/full
                 │   │ of ammo │      ▼       ▼
                 │   │         │  ┌──────────────┐
                 │   └─────────┤  │ ReceiveHeal  │
                 └─────────────┤  │ ReceiveAmmo  │
                               │  └──────────────┘
                               └──── back to SeekEnemy
```

**Priority order:** Critical HP → Engage → Low HP → Low Ammo → Seek

### Support FSM (Medic & Supplier)

```
   ┌──────────────┐      supply low     ┌───────────────┐
   │ IdleSupport  │─────────────────────►│ SelfMaintain  │
   │ (find client)│                      │ (go to depot) │
   └──────┬───────┘                      └───────────────┘
          │ client found
          ▼
   ┌──────────────┐      in range       ┌───────────────┐
   │ MoveToClient │─────────────────────►│  ServeClient  │
   │              │                      │ (heal/supply) │
   └──────────────┘                      └───────────────┘
          │ enemy nearby, no escort
          ▼
   ┌──────────────┐
   │  FleeState   │
   │ (run to safe)│
   └──────────────┘
```

### Personality Traits

Each agent is initialized with randomized personality values that affect decision-making:

| Trait | Range | Effect |
|---|---|---|
| `aggression` | 0.2–0.8 | Higher → more grenade usage in combat |
| `riskAversion` | 0.2–0.8 | Higher → A\* avoids risky cells/rooms more |
| `ammoThreshold` | 3–10 | Below this → triggers resupply request |
| `healthThreshold` | 20–49 | Below this → triggers healing request |

---

## Dungeon Generation

1. **Room Placement** — 8 rooms with random size (8–22 tiles) placed without overlap on a 100×100 grid
2. **Corridor Carving** — A\* connects every room pair; wall traversal costs more, so paths reuse existing open areas
3. **Obstacle Scatter** — Random obstacles placed inside rooms for cover
4. **Depot Assignment** — 2 ammo depots (🟡) and 2 medical depots (🟢) placed in random rooms
5. **Team Spawning** — Rooms sorted by X-position; left 4 rooms → Blue team, right 4 rooms → Red team

---

## Class Reference

### Core Classes

| Class | File | Description |
|---|---|---|
| `BaseAgent` | `BaseAgent.h/cpp` | Abstract base for all agents. Handles movement, A\* cell pathfinding, damage, room detection, and state management |
| `Fighter` | `Fighter.h/cpp` | Combat unit with ammo, attack cooldown, bullet/grenade firing, and support request flags |
| `Medic` | `Medic.h/cpp` | Heals wounded fighters. Refills healing supply at medical depots |
| `Supplier` | `Supplier.h/cpp` | Resupplies ammo to fighters. Refills at ammo depots |
| `Team` | `Team.h/cpp` | Container for a team's agents (2 fighters, 1 medic, 1 supplier) |

### World Classes

| Class | File | Description |
|---|---|---|
| `Room` | `Room.h/cpp` | Dungeon room with position, size, depot type, obstacles, and room connections |
| `Cell` | `Cell.h/cpp` | A\* pathfinding node with F/G/H costs and parent pointer |
| `SafetyMap` | `SafetyMap.h/cpp` | Combat risk heat map (per-cell and per-room) with decay |
| `Bullet` | `Bullet.h/cpp` | Projectile movement and rendering |
| `Grenade` | `Grenade.h/cpp` | AoE explosion using 20 bullet fragments |

### State Classes

| Class | File | Agent Type | Description |
|---|---|---|---|
| `SeekEnemy` | `FighterStates` | Fighter | Navigate toward nearest enemy |
| `EngageCombat` | `FighterStates` | Fighter | Fire bullets or throw grenades at visible enemies |
| `MoveToMedic` | `FighterStates` | Fighter | Navigate to team's medic when low HP |
| `MoveToSupplier` | `FighterStates` | Fighter | Navigate to team's supplier when low ammo |
| `ReceiveHeal` | `FighterStates` | Fighter | Wait in place while medic heals |
| `ReceiveAmmo` | `FighterStates` | Fighter | Wait in place while supplier resupplies |
| `IdleSupport` | `SupportStates` | Medic/Supplier | Scan for clients; follow nearest fighter |
| `MoveToClient` | `SupportStates` | Medic/Supplier | Navigate to assigned client |
| `ServeClient` | `SupportStates` | Medic/Supplier | Heal or resupply the client in range |
| `SelfMaintain` | `SupportStates` | Medic/Supplier | Navigate to depot to restock supplies |
| `FleeState` | `SupportStates` | Medic/Supplier | Flee to safest room when enemies are near and no escort |

### Other

| File | Description |
|---|---|
| `Globals.h` | All game constants (grid size, speeds, damage, limits) |
| `Personality.h` | Struct with randomized personality traits for each agent |
| `State.h` | Abstract FSM state interface (`OnEnter`, `Execute`, `OnExit`) |
| `CompareCells.h` | Comparator for A\* priority queue (min-F ordering) |

---

## Build & Run

### Prerequisites

- **Visual Studio 2019+** (with C++ Desktop Development workload)
- Libraries included in the project directory:
  - `freeglut.lib` / `freeglut.h`
  - `glew32.lib` / `glew.h`

### Steps

1. Clone the repository:
   ```bash
   git clone https://github.com/your-username/AI-Team-Battle-Game.git
   ```
2. Open `Graphics.sln` in Visual Studio
3. Set the build configuration to **Debug x64** (or your preferred target)
4. Build and run (**F5** or **Ctrl+F5**)

> **Note:** `freeglut.dll` and `glew32.dll` must be accessible at runtime. Place them next to the executable or in a system PATH directory.

---

## Controls

| Input | Action |
|---|---|
| **Right-click** | Opens context menu |
| **Restart Game** (menu) | Regenerates dungeon and resets all teams |

The simulation runs autonomously — no manual control over agents.

---

## Configuration

All game parameters are defined in [`Globals.h`](Globals.h) and can be tuned:

```cpp
// Grid & Layout
const int MSZ = 100;              // Maze grid size (100×100)
const int NUM_ROOMS = 8;          // Number of dungeon rooms

// Movement
const double AGENT_SPEED = 0.08;  // Fighter movement speed
const double SUPPORT_SPEED = 0.14;// Medic/Supplier speed (faster)
const double BULLET_SPEED = 0.3;  // Visual bullet travel speed

// Combat
const double BULLET_DAMAGE = 10.0;
const double GRENADE_DAMAGE = 5.0;
const int ATTACK_COOLDOWN = 15;   // Ticks between attacks
const int GRENADE_AMMO_COST = 3;  // Ammo per grenade throw

// Resources
const int MAX_HEALTH = 100;
const int FIGHTER_MAX_AMMO = 30;
const int FIGHTER_START_AMMO = 15;
const int MEDIC_MAX_SUPPLY = 30;
const int SUPPLIER_MAX_SUPPLY = 30;
```

---

## 📁 Project Structure

```
Graphics/
├── main.cpp                 # Game loop, rendering, dungeon generation
├── Globals.h                # All constants and enums
├── BaseAgent.h/cpp          # Abstract agent base class
├── Fighter.h/cpp            # Combat agent
├── Medic.h/cpp              # Healing support agent
├── Supplier.h/cpp           # Ammo supply agent
├── Team.h/cpp               # Team container
├── FighterStates.h/cpp      # Fighter FSM states + utility functions
├── SupportStates.h/cpp      # Medic/Supplier FSM states
├── State.h/cpp              # Abstract state interface
├── Personality.h            # Randomized personality traits
├── Room.h/cpp               # Dungeon room
├── Cell.h/cpp               # A* pathfinding node
├── CompareCells.h/cpp       # A* priority queue comparator
├── SafetyMap.h/cpp          # Combat risk heat map
├── Bullet.h/cpp             # Bullet projectile
├── Grenade.h/cpp            # Grenade (AoE explosion)
├── Graphics.sln             # Visual Studio solution
├── Graphics.vcxproj         # Visual Studio project
├── freeglut.h / .lib        # FreeGLUT library
├── glew.h / glew32.lib      # GLEW library
└── glut.h                   # GLUT header wrapper
```

---

## License

This project was developed as an academic exercise in AI game simulation and computer graphics.
