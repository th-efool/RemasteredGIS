# RemasteredGIS

# 🌍 Unreal GIS Plugin

**Unreal GIS Plugin** is a feature-rich Unreal Engine module for real-time visualization of real-world geospatial data.  
It bridges **GIS datasets** and **game-engine rendering pipelines**, enabling developers to create immersive, data-driven 3D worlds and AR navigation systems directly inside Unreal Engine.

---

## 🧭 Core Features

- **🗺️ Real-World Tile Streaming**
  - Dynamic 4×4 background tile grid with 3×3 active viewport window.
  - Seamless tile replacement on pan or zoom — constant memory footprint.
  - Asynchronous fetch, caching, and eviction for high-performance streaming.

- **📍 GIS Integration**
  - Supports coordinate conversion (WGS84 ↔ UE world space).
  - Flexible projection layer — plug in custom map projections or datum shifts.
  - Easily integrate with REST or local tile servers (Mapbox, OSM, ESRI, etc.).

- **🧠 Scalable Architecture**
  - Modular design with `Editor` and `Runtime` modules.
  - Clean data flow: `TileFetcher → TileCache → TileRenderer`.
  - Event-driven refresh for efficient viewport updates.

- **📱 AR Navigation (Experimental)**
  - Aligns Unreal world transforms with GPS/IMU data.
  - Real-time position anchoring for AR and mixed-reality use cases.

- **💡 Extensible**
  - Expose plugin services through Blueprints and C++.
  - Optional custom materials for terrain, imagery, and vector overlays.

---

## ⚙️ Architecture Overview

```text
┌─────────────────────────────┐
│ Unreal GIS Plugin           │
│ (Editor + Runtime)          │
├───────────────┬─────────────┤
│ TileFetcher   │ Fetches raster/vector tiles from source
│ TileCache     │ Stores active + background tiles
│ TileRenderer  │ Converts tiles into UTexture + meshes
│ CoordSystem   │ Handles GIS <-> Unreal conversions
│ ARSyncModule  │ Integrates device GPS/IMU with UE transforms
└───────────────┴─────────────┘
```
