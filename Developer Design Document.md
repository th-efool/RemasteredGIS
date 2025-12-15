
# 📐 GIS Renderer Plugin

**Developer Design Document (v0.1)**
## 0. Introduction & Project Overview

The **GIS Renderer Plugin** is a real-time geospatial rendering and navigation engine that streams, renders, and augments real-world geographic data inside interactive applications.

It is designed as a **spatial runtime**, not a traditional map viewer.

The system allows applications to load and interact with real-world geography dynamically. Geographic data is streamed on demand and rendered as raster imagery, vector map data, or elevation-based terrain. All rendering modes share the same coordinate system and spatial logic, ensuring consistency across zoom levels, projections, and navigation states.

To support large geographic areas, the system uses a **fixed-size tile streaming model**. Only a bounded set of tiles is kept in memory at any time. As the camera moves, tiles entering the view are fetched and tiles leaving the view are evicted in a deterministic way. This keeps memory usage stable regardless of the size of the mapped area.

Navigation is built into the core runtime rather than layered on top. Routes are fetched from provider-agnostic APIs and parsed into structured route data. A live _Journey_ system tracks progress along the route using the current user location, updating the active path, remaining distance, estimated time, and navigation instructions continuously.

All spatial computation—coordinate conversion, routing logic, geocoding, and tile addressing—is handled inside the GIS core. Latitude and longitude remain the authoritative coordinate space throughout the system. Conversion into engine-specific world coordinates happens only at the point of rendering to avoid precision loss and inconsistencies.

The architecture is **engine-independent and provider-independent**. The current integration targets:

- **Godot** as the rendering and input layer
    
- **Rust** as the GIS runtime responsible for streaming, navigation, geocoding, and spatial computation
    

Rust is used to provide predictable memory behavior, safe concurrency for asynchronous data fetching, and accurate geographic calculations. Godot is responsible only for rendering, user input, and presentation. Communication between the two is handled through a strict FFI boundary with clear ownership and lifetime rules.

This document is a **technical design specification**. It describes the system as it exists today—tile streaming, quadtree management, navigation journeys, overlays, and asynchronous data pipelines—and formalizes the design to support continued development and extension.

Alright.  
I’ll treat this as a **proper internal engineering design doc**, not marketing fluff, not README-level, and not UE-specific.  
Platform-agnostic, **Godot + Rust target**, but cleanly abstracted so the renderer could be embedded elsewhere later.

I’ll also **implicitly reflect the actual system you already built** (tile streaming, quadtree, navigation, overlay, async APIs) instead of reinventing it.

---
## 1. Purpose & Scope

The **GIS Renderer Plugin** is a real-time geospatial rendering and navigation engine that streams, renders, and augments real-world geographic data inside an interactive 2D/3D scene.

It is designed as a **core spatial runtime**, not a map viewer.

Primary goals:
- Render real-world geography (raster, vector, elevation) interactively
- Maintain **constant memory usage** via tile streaming
- Provide **navigation + geocoding** as first-class primitives
- Support **AR-style world overlays**
- Remain **provider-agnostic** and **engine-agnostic**
- Move heavy logic into **Rust** (deterministic, safe, testable)

Target integration (current):
- **Godot Engine (frontend / renderer)**
- **Rust (core GIS + data + navigation engine)**

---

## 2. High-Level Architecture
```text
┌──────────────────────────────┐
│      Host Engine (Godot)     │
│  - Camera / Viewport         │
│  - Mesh & Material Binding   │
│  - Input (pan / zoom / AR)   │
└───────────────▲──────────────┘
                │ FFI / Bindings
┌───────────────┴──────────────┐
│     GIS Renderer Core (Rust) │
│                              │
│  ┌──────── Tile System ────┐ │
│  │ Quadtree / Tile IDs     │ │
│  │ Streaming Window        │ │
│  │ Cache & Eviction        │ │
│  └────────────────────────┘ │
│                              │
│  ┌──── Rendering Models ───┐ │
│  │ Raster Tiles             │
│  │ Vector Tiles (MVT)       │
│  │ Height / DEM Tiles       │
│  └────────────────────────┘ │
│                              │
│  ┌──── Navigation Engine ──┐ │
│  │ Route Fetching           │
│  │ Path Flattening          │
│  │ Journey State            │
│  │ Instructions             │
│  └────────────────────────┘ │
│                              │
│  ┌──── Geo Intelligence ───┐ │
│  │ Lat/Lon ↔ Tile           │
│  │ Geocoding / Reverse      │
│  │ POI / Overlay Data       │
│  └────────────────────────┘ │
└──────────────────────────────┘
```

---

## 3. Core Design Principles

### 3.1 Constant-Memory Streaming
- World is infinite; memory is not.
- Only a **fixed window of tiles** exists in memory.
- Default strategy:
    - **4×4 atlas grid**
    - **3×3 active viewport**
- Panning shifts the window; tiles are evicted and fetched deterministically.

### 3.2 Data-Driven Everything
- No static markers.
- No baked paths.
- Navigation, markers, overlays all react to **live state**:
    - user location
    - zoom level
    - provider data
    - journey progress

### 3.3 Precision First
- All geographic math is **double-precision**.
- Rendering coordinates are derived _last_.
- No lossy projection early in the pipeline.

---
## 4. Tile System

### 4.1 Tile Identity
```rust
struct TileId {
    zoom: u8,
    x: u32,
    y: u32,
}
```
- Slippy-map compatible
- Hashable
- Parent/child relationships (quadtree)

### 4.2 Quadtree

Responsibilities:
- Logical tile hierarchy
- Cache ownership
- Parent/child resolution
- LOD transitions (future)

Each node:
```rust
struct TileNode<T> {
    id: TileId,
    resource: Option<T>,
    fetched: bool,
    children: [Option<NodeRef>; 4],
}
```

### 4.3 Tile Streaming Window

Inputs:
- Center tile
- Grid dimensions
- Camera offset (fractional)

Outputs:
- Deterministic list of visible TileIds
- Fetch / evict decisions

This logic already exists in your system and **must remain deterministic** to avoid jitter, flicker, or tile drift.

---
## 5. Rendering Modes

### 5.1 Static Raster Tiles
- PNG / JPG
- Satellite / imagery
- Decoded in Rust
- Uploaded as engine textures

Fallback:
- Solid color tile for missing data
### 5.2 Vector Tiles
- MVT (Mapbox Vector Tile)
- Parsed in Rust
- Geometry flattened into:
    - lines (roads)
    - polygons (buildings)
    - points (POIs)
Renderer decides:
- mesh generation
- styling
- labels

### 5.3 Height / DEM Tiles
- Height encoded (PNG / GeoTIFF / quantized mesh)
- Converted into height arrays
- Mesh generated per tile
- Stitching & skirts handled at tile boundaries

---

## 6. Coordinate Conversion Engine
This is **foundational** and already a major solved pain point.

### 6.1 Supported Spaces
- Lat / Lon (WGS84)
- Tile Space (X/Y/Z)
- Local Tile Space (fractional offsets)
- Engine World Space

### 6.2 Core Functions

```rust
latlon_to_tile(lat, lon, zoom) -> TileId
tile_to_latlon(tile, offset) -> LatLon
local_to_gis(local_point, camera_state) -> LatLon
gis_to_local(latlon, camera_state) -> Vec2
```

### 6.3 Invariants
- Center tile must always map to viewport center.
- Camera offsets must be normalized against grid bounds.
- Odd/even grid parity must be preserved.

Your previous debug-trace heavy work **stays** — it’s essential.

---
## 7. Navigation System

### 7.1 Route Fetching
- Provider-agnostic API layer
- URL built dynamically:
    - origin
    - destination
    - travel mode

Rust owns:
- HTTP
- JSON parsing
- error handling

### 7.2 Route Data Model

```rust
struct GeoCoordinate {
    lat: f64,
    lon: f64,
}

struct RouteStep {
    instruction: String,
    distance_m: f64,
    duration_s: f64,
    geometry: Vec<GeoCoordinate>,
}

struct Route {
    steps: Vec<RouteStep>,
    geometry: Vec<GeoCoordinate>, // flattened
}
```

### 7.3 Journey System

A **Journey** is a live navigation session.

```rust
struct Journey {
    route: Route,
    current_index: usize,
    remaining_distance: f64,
    remaining_duration: f64,
}
```

- Advances based on user location
- Emits:
    - current instruction
    - remaining path
    - ETA updates

This replaces static path rendering entirely.

---
## 8. Overlay & Marker System

### 8.1 Marker Model
```rust
struct Marker {
    lat: f64,
    lon: f64,
    kind: MarkerType,
}
```

Markers:
- Stored in GIS space
- Reprojected every frame
- Auto-hidden when out of relevance
### 8.2 Path Rendering
- Spline / polyline generated from route geometry
- Clipped to local area bounds
- Truncated, never “disappears”
- Preserves visual continuity

(Your spline clipping solution is **correct and intentional**.)

---
## 9. Geocoding & World Intelligence

### 9.1 Forward / Reverse Geocoding
- Provider API or local DB
- Rust returns structured results
- Engine decides presentation
### 9.2 Custom World Data
- User-defined POIs
- Metadata overlays
- Reviews, ratings, tags
- Used heavily in AR mode
---
## 10. AR Overlay Mode (Conceptual)
- GIS world is anchor truth
- Device pose is relative
- Overlays projected from GIS → camera
- Same marker + journey systems reused

No separate AR logic branch.

---
## 11. Error Handling & Logging
- Centralized logging (already built)
- Severity-aware
- Engine-filterable
- Safe in release builds

This is **non-negotiable** for a system this complex.

---
## 12. Why Rust + Godot (Explicit Rationale)
Rust:
- Deterministic memory
- Safe async
- Testable math
- No engine lifecycle footguns

Godot:
- Lightweight renderer
- Easy FFI
- Mobile & desktop friendly
- AR support without vendor lock-in

---

## 13. Non-Goals (Explicit)

- Not a UI map SDK
- Not a static map exporter
- Not provider-locked
- Not GoDot-only
- Not “just tiles”
----
# 🔌 Godot ↔ Rust FFI ABI Design

## 0. Core Philosophy (non-negotiable)
1. **Rust owns all logic and state**
2. **Godot owns all rendering objects**
3. **No shared mutable memory**
4. **No callbacks from Rust into Godot**
5. **Godot pulls snapshots; Rust never pushes**

If you violate any of these, the project will rot.

---
## 1. ABI Shape (C-compatible, stable)

We expose a **flat C ABI** from Rust.
- No Rust structs exposed directly
- No lifetimes across boundary
- No async across boundary
- No Godot types in Rust
- No Rust types in Godot
### Language boundary
```text
Godot (GDExtension / C++)
        ↓
     C ABI
        ↓
      Rust
```
---

## 2. Opaque Handles (Ownership Model)

### Rust side

```rust
#[repr(C)]
pub struct RuntimeHandle {
    ptr: *mut Runtime,
}
```

- Godot never dereferences this
- Godot treats it as an opaque token
- Rust guarantees validity until destroyed
### Godot side
```cpp
typedef void* GIS_Runtime;
```

---

## 3. Lifecycle API (Minimal & Complete)

### 3.1 Create / Destroy

```c
GIS_Runtime gis_runtime_create();
void gis_runtime_destroy(GIS_Runtime runtime);
```

**Rules**

- `create` allocates all long-lived Rust state
- `destroy` stops async tasks, drains queues, frees memory
- Godot must call `destroy` exactly once
---

## 4. Configuration Phase (Before Start)
No hot mutation of core config.
```c
void gis_runtime_set_provider(
    GIS_Runtime runtime,
    const char* provider_name,
    const char* api_key
);

void gis_runtime_set_viewport(
    GIS_Runtime runtime,
    int grid_x,
    int grid_y
);
```

Rules:
- Strings are **copied immediately**
- Caller retains ownership of strings
- Configuration must happen **before first update**
---
## 5. Update Loop Contract (Heart of the System)

### 5.1 Godot → Rust (Push minimal state)

Called **once per frame**.
```c
void gis_runtime_update(
    GIS_Runtime runtime,
    double delta_time,
    double camera_lat,
    double camera_lon,
    double zoom,
    double heading_deg
);
```

Rust responsibilities:
- advance streaming window
- advance journey state
- poll async channels
- update internal world state
❗ **No rendering happens here**

---

## 6. Snapshot Model (Pull, Don’t Push)
Godot pulls **immutable snapshots** every frame.

### 6.1 Tile Snapshot
```c
typedef struct {
    int zoom;
    int x;
    int y;
    int kind;      // raster / vector / height
    const void* data;
    int data_len;
} GIS_TileSnapshot;
```

```c
int gis_runtime_get_visible_tiles(
    GIS_Runtime runtime,
    const GIS_TileSnapshot** out_tiles
);
```

Rules:
- Rust owns memory
- Snapshot valid **until next update**
- Godot must copy data if it wants to keep it
---

### 6.2 Navigation Snapshot
```c
typedef struct {
    double lat;
    double lon;
} GIS_Point;

typedef struct {
    GIS_Point* points;
    int count;
} GIS_Path;
```

```c
bool gis_runtime_get_active_path(
    GIS_Runtime runtime,
    GIS_Path* out_path
);
```

- Geometry already flattened
- Ordered for spline / polyline rendering
- Valid for one frame only
---

### 6.3 Instruction Snapshot
```c
typedef struct {
    const char* text;
    double remaining_distance_m;
    double remaining_time_s;
} GIS_NavInstruction;
```

```c
bool gis_runtime_get_current_instruction(
    GIS_Runtime runtime,
    GIS_NavInstruction* out_instruction
);
```

Strings:
- UTF-8
- Rust-owned
- Valid until next `update()`
---

## 7. Marker / Overlay Snapshots
```c
typedef struct {
    double lat;
    double lon;
    int kind;
} GIS_Marker;
```

```c
int gis_runtime_get_visible_markers(
    GIS_Runtime runtime,
    const GIS_Marker** out_markers
);
```

Markers:
- Always stored in GIS space
- Godot projects them into world/AR
---

## 8. Memory Rules (READ THIS TWICE)

### 8.1 Allocation Rules

|Resource|Owner|
|---|---|
|Runtime|Rust|
|Tile data|Rust|
|Route geometry|Rust|
|Strings|Rust|
|Meshes|Godot|
|Textures|Godot|

### 8.2 Lifetime Rules
- All snapshot pointers:
    - **invalidated on next `update()`**
- Godot must copy if persistence is needed
- Rust never frees memory mid-frame
### 8.3 Threading Rules
- All ABI calls are **single-threaded**
- Rust async tasks communicate via channels
- Godot thread never blocks on Rust async

## 9. Error Handling Strategy
No panics across FFI.
```c
int gis_runtime_get_last_error(
    GIS_Runtime runtime,
    const char** out_message
);
```

- Errors are sticky until queried
- Godot can log or surface them
- Rust continues running unless fatal
## 10. Minimal Godot Frame Pseudocode
```cpp
void _process(double dt) {
    gis_runtime_update(
        runtime,
        dt,
        camera_lat,
        camera_lon,
        zoom,
        heading
    );

    // tiles
    const GIS_TileSnapshot* tiles;
    int count = gis_runtime_get_visible_tiles(runtime, &tiles);
    render_tiles(tiles, count);

    // navigation
    GIS_Path path;
    if (gis_runtime_get_active_path(runtime, &path)) {
        render_path(path);
    }

    // instruction
    GIS_NavInstruction instr;
    if (gis_runtime_get_current_instruction(runtime, &instr)) {
        ui_show(instr.text);
    }
}
```

No callbacks.  
No surprises.  
No UB.

## 11. Why This ABI Will Survive
- Godot can crash → Rust still clean
- Rust can restart runtime → Godot unaffected
- Async never leaks
- No lifetime ambiguity
- No engine lock-in

This is **exactly** how engines like Unity DOTS, Mapbox Native, and game physics runtimes do it internally.

---

# 🦀 Rust Module Layout

## 1. Crate Topology

This should **not** be one monolithic crate.

```text
gis/
├── crates/
│   ├── gis-core/        # Pure GIS math + types (NO async, NO IO)
│   ├── gis-tiles/       # Tile streaming, quadtree, cache
│   ├── gis-net/         # HTTP, providers, async fetchers
│   ├── gis-nav/         # Routing, journeys, navigation logic
│   ├── gis-geo/         # Geocoding, POIs, world intelligence
│   ├── gis-runtime/    # Orchestrator / facade used by engines
│   ├── gis-ffi/        # Godot <-> Rust boundary
│   └── gis-log/        # Logging & diagnostics
│
└── Cargo.toml           # Workspace
```

**Rule:**

- Anything that touches the network → async → `gis-net`
- Anything that touches the engine → `gis-ffi`
- Anything that does math → `gis-core`
- Anything that _coordinates systems_ → `gis-runtime`

---

## 2. Core Crates (Deep Dive)

### 2.1 `gis-core` (Zero-dependency foundation)

**NO async. NO IO. NO engine.**
```text
gis-core/
├── lib.rs
├── geo/
│   ├── latlon.rs
│   ├── tile_id.rs
│   ├── projection.rs
│   └── bounds.rs
├── math/
│   ├── mercator.rs
│   └── precision.rs
└── traits/
    ├── provider.rs
    └── render_model.rs
```

#### Responsibilities
- `LatLon`, `TileId`, `GeoBounds`
- Mercator math
- Tile ↔ lat/lon conversions
- **Traits only**, no implementations

```rust
pub struct LatLon {
    pub lat: f64,
    pub lon: f64,
}

pub struct TileId {
    pub zoom: u8,
    pub x: u32,
    pub y: u32,
}
```

This crate must be:
- deterministic
- testable
- engine-agnostic
---
### 2.2 `gis-tiles` (Streaming + quadtree)
```text
gis-tiles/
├── lib.rs
├── quadtree/
│   ├── node.rs
│   ├── tree.rs
│   └── id.rs
├── cache/
│   ├── lru.rs
│   └── entry.rs
├── streaming/
│   ├── window.rs
│   └── planner.rs
└── resources/
    ├── raster.rs
    ├── vector.rs
    └── height.rs
```
#### Responsibilities
- Tile window calculation (3×3, 4×4, etc.)
- Cache eviction
- Tile lifecycle state:
    - requested
    - in-flight
    - ready
    - evicted
```rust
pub enum TileState {
    Empty,
    Fetching,
    Ready,
}
```

❗ **Important**
- `gis-tiles` does **not** fetch data.
- It only _asks_ for tiles.
Fetching is injected.

---
### 2.3 `gis-net` (Async, providers, HTTP)
```text
gis-net/
├── lib.rs
├── http/
│   ├── client.rs
│   └── error.rs
├── providers/
│   ├── mapbox.rs
│   ├── osm.rs
│   └── traits.rs
├── fetchers/
│   ├── raster.rs
│   ├── vector.rs
│   ├── height.rs
│   └── route.rs
└── decode/
    ├── image.rs
    ├── mvt.rs
    └── dem.rs
```

#### Responsibilities
- Async HTTP
- Provider-specific URL building
- Decoding network payloads → raw data models
```rust
#[async_trait]
pub trait TileProvider {
    async fn fetch_raster(&self, id: TileId) -> Result<RasterTile>;
}
```

Uses:
- `tokio`
- `reqwest`
- `serde`
Nothing engine-facing.

---
### 2.4 `gis-nav` (Navigation engine)

```text
gis-nav/
├── lib.rs
├── route/
│   ├── model.rs
│   ├── parser.rs
│   └── flatten.rs
├── journey/
│   ├── journey.rs
│   └── progress.rs
└── instructions/
    └── turn.rs
```
#### Responsibilities
- Route parsing
- Geometry flattening
- Journey state machine
```rust
pub struct Journey {
    route: Route,
    cursor: usize,
}
```

No rendering.  
No HTTP.  
Pure logic + async entry points.

---
### 2.5 `gis-geo` (Geocoding & POIs)

```text
gis-geo/
├── lib.rs
├── geocode/
│   ├── forward.rs
│   └── reverse.rs
├── poi/
│   ├── model.rs
│   └── index.rs
└── overlay/
    └── metadata.rs
```

Supports:
- External geocoding APIs
- Local POI DBs
- AR overlay metadata
---
## 3. `gis-runtime` (The brain)

This is the **only crate the engine talks to**.
```text
gis-runtime/
├── lib.rs
├── state/
│   ├── camera.rs
│   ├── viewport.rs
│   └── world.rs
├── systems/
│   ├── tile_system.rs
│   ├── nav_system.rs
│   └── overlay_system.rs
└── facade.rs
```

### Responsibilities
- Hold long-lived state
- Glue tiles + nav + geo together
- Emit **plain data outputs** for renderer
```rust
pub struct Runtime {
    tile_system: TileSystem,
    nav_system: NavSystem,
}
```

This is where:
- tile requests are scheduled
- async results are applied
- world state advances
---
## 4. `gis-ffi` (Godot boundary)
```text
gis-ffi/
├── lib.rs
├── api/
│   ├── runtime.rs
│   ├── tiles.rs
│   └── navigation.rs
├── types/
│   ├── vectors.rs
│   └── handles.rs
└── ownership.rs
```

### Rules
- No internal structs leak
- Engine sees **opaque handles**
- Rust owns memory
```rust
#[repr(C)]
pub struct RuntimeHandle(*mut Runtime);
```

Godot:
- calls `update(dt)`
- pulls snapshot data
- never touches internals
---
## 5. Ownership Boundaries (Critical)
### 5.1 What Rust Owns
- Tile cache
- Route data
- Navigation state
- Geo math
- Async tasks
### 5.2 What Engine Owns
- Meshes
- Materials
- Textures
- UI widgets
### 5.3 Data Flow Direction
```text
Network → Rust → Plain Data → Engine → GPU
```

Never:
```text
Engine → Rust → GPU
```

---
## 6. Async Model (Very Important)

### 6.1 Single Tokio Runtime
- Created once (inside `gis-runtime`)
- Shared via handles

### 6.2 Async Boundaries

|Layer|Async?|
|---|---|
|gis-core|❌|
|gis-tiles|❌|
|gis-net|✅|
|gis-nav|⚠️ (fetch only)|
|gis-runtime|⚠️ (orchestration)|
|gis-ffi|❌|

### 6.3 Pattern Used

- **Command → async task → channel → apply on tick**
```rust
// async task
tokio::spawn(async move {
    let tile = provider.fetch_raster(id).await?;
    tx.send(Event::TileReady(tile));
});
```

Runtime:
- drains channel during `update()`
- applies changes deterministically

No async inside render loop.

---

## 7. Why This Layout Will Scale
- You can:
    - swap providers
    - swap engines
    - add offline routing
    - add LOD tiles
- without touching core math
- without rewriting FFI
- without async bleeding into rendering
