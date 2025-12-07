# Advanced Tools – Physics Performance Comparison (Unity vs Unreal Engine)

## Introduction
This project compares the **physics simulation performance** of **Unity 6.2 (PhysX)** and **Unreal Engine 5.7 (Chaos)**.  
The test evaluates how each engine handles an increasing number of physics-enabled cubes under identical test conditions.

The goals were to measure:
- Average **Frame Time (ms)** and scalability with object count  
- **Memory usage** during the simulation  
- **Stability**, represented by how long it takes for all objects to stop simulating physics  

---

## Test Setup

Both Unity and Unreal scenes were built to be **functionally identical** for a fair comparison:

- A **100×100 cube** was used as the floor.  
- A **camera** was positioned to view the spawned cubes.  
- A simple **UI** was included with:
  - An input field to specify the number of cubes to spawn.  
  - A "Spawn" button to initiate the stress test.  

Each test spawned between **500 and 10,000 cubes** arranged in a 10×N grid, spaced evenly apart.

Physics properties were matched as closely as possible:
| Property | Value |
|:--|:--|
| Mass | 1.0 kg |
| Friction | 0.6 |
| Restitution (Bounciness) | 0.0 |
| Gravity | Enabled (default in both engines) |

Simulations ran for **30 seconds**, with data logged every frame.

### Recorded Metrics
| Metric | Description |
|:--|:--|
| **FrameTime (ms)** | Time taken to render a single frame |
| **TotalMemory (MB)** | Physical memory used |
| **ActiveObjects** | Rigidbodies still awake and simulating physics |

---

## Scene Implementation

### Unity (PhysX)
- Script: `PhysicsStressTest.cs`  
- Used `ProfilerRecorder` to track Physics Time and Total Memory.  
- Spawned **20 cubes per frame** to avoid freezing.  
- Logged all metrics to `PhysicsData_Combined.csv`.

### Unreal (Chaos)
- Script: `PhysicsStressTest.cpp/.h`  
- Used `FPlatformMemory::GetStats()` and `UPrimitiveComponent::IsAnyRigidBodyAwake()`.  
- Spawned **20 actors per frame** to match Unity’s behavior.  
- Logged results to `UnrealPhysicsData.csv`.

---

## Results & Comparison

### Time Until Active Objects Reached Zero (Stability)

<p align="center">
  <img src="charts/unreal_stability1.png" width="800"/>
  <br>
  <em>Figure 1 – Unreal Engine: Time until all cubes stop simulating physics</em>
</p>

<p align="center">
  <img src="charts/unity_stability1.png" width="800"/>
  <br>
  <em>Figure 2 – Unity: Time until all cubes stop simulating physics</em>
</p>

#### Analysis
<p>
In Unity, the cubes begin to fall and settle quickly, even at lower object counts. This leads to a faster drop in active rigidbodies as the simulation stabilizes. 
Unity’s PhysX implementation tends to resolve stacking instability more aggressively, allowing objects to enter a sleeping state sooner.
<br><a href="https://docs.unity3d.com/Manual/PhysicsSection.html" target="_blank">Unity Manual – Physics (PhysX Integration)</a>
</p>
<p>
In Unreal Engine, the Chaos physics solver maintains more accurate constraint and contact resolution. 
For tests with 500 and 1000 cubes, stable towers remained upright for the entire 30-second duration, preventing the active object count from reaching zero. 
At 2000 cubes, the increased pile density caused the towers to collapse and the engine managed to stabilize after some time. 
For 5000 and 10000 cubes, the towers again collapsed; however, due to the high number of objects and the resulting complex interactions, the engine was 
unable to fully stabilize within the 30-second test window.
This reflects Chaos Physics’ emphasis on stability and realism, even when it results in longer simulation activity.
<br><a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine" target="_blank">Unreal Engine Documentation – Physics (Chaos)</a>
</p>

<p align="center">
  <img src="charts/unreal_500_towers.png" width="380"/>
  <img src="charts/unity_500_cubes.png" width="380"/><br>
  <em>Figure A – Unreal 500 vs Unity 500 – Tower formation in Unreal versus collapsed stacks in Unity</em>
</p>

<p align="center">
  <img src="charts/unreal_1000_towers.png" width="380"/>
  <img src="charts/unity_1000_cubes.png" width="380"/><br>
  <em>Figure B – Unreal 1000 vs Unity 1000 – Towers remaining upright in Unreal, fully collapsed in Unity</em>
</p>

This demonstrates a key difference:
Unreal’s Chaos Physics emphasizes **physical realism and stability**, while Unity’s PhysX favors **simplified and more easily reduced interactions**.

---

### Frame Time vs Object Count

<p align="center">
  <img src="charts/unreal_frametime1.png" width="800"/>
  <br>
  <em>Figure 3 – Unreal Engine: Average frame time (ms) vs object count</em>
</p>

<p align="center">
  <img src="charts/unity_frametime1.png" width="800"/>
  <br>
  <em>Figure 4 – Unity: Average frame time (ms) vs object count</em>
</p>

| Object Count | Unity FrameTime (ms) | Unreal FrameTime (ms) |
|:--:|:--:|:--:|
| 500 | 1.47 | 4.43 |
| 1000 | 2.29 | 4.69 |
| 2000 | 3.77 | 6.33 |
| 5000 | 8.99 | 95.51 |
| 10000 | 23.95 | 314.24 |

#### Analysis
<p>
Unity’s PhysX maintains near-linear scaling and performs efficiently even with thousands of rigidbodies. 
This is achieved through broad-phase algorithms such as sweep-and-prune, which quickly filter out non-colliding bodies before performing detailed checks. 
The result is consistent frame times and minimal CPU overhead per object.
<br><a href="https://docs.unity3d.com/Manual/PhysicsSection.html" target="_blank">Unity Manual – Physics (PhysX Integration)</a> • 
<a href="https://docs.unity3d.com/Manual/PhysicsOptimization.html" target="_blank">Unity Manual – Physics Optimization</a>
</p>

<p>
Unreal Engine’s Chaos Physics shows a steeper increase in frame time as object count rises. 
Chaos provides a unified, high-fidelity simulation system for rigid bodies, collisions, and constraints, which trades performance for realism and stability. 
Every body participates fully in constraint solving and contact management, leading to higher computational cost with large numbers of objects.
<br><a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine" target="_blank">Unreal Engine Documentation – Physics (Chaos)</a>
</p>

<p>
Overall, Unity favors performance-optimized batching, while Unreal prioritizes more physically accurate interactions.
</p>

### Memory Usage vs Object Count

<p align="center">
  <img src="charts/unreal_memory1.png" width="800"/>
  <br>
  <em>Figure 5 – Unreal Engine: Average memory usage (MB) vs object count</em>
</p>

<p align="center">
  <img src="charts/unity_memory1.png" width="800"/>
  <br>
  <em>Figure 6 – Unity: Average memory usage (MB) vs object count</em>
</p>

| Object Count | Unity (MB) | Unreal (MB) |
|:--:|:--:|:--:|
| 500 | 254 | 1652 |
| 1000 | 261 | 1664 |
| 2000 | 276 | 1707 |
| 5000 | 316 | 1813 |
| 10000 | 376 | 2029 |

#### Analysis
<p>
Unreal Engine consumes <b>6–8× more memory</b> than Unity in these tests. A major factor is how Unreal represents and manages simulation objects:
</p>

<ul>
  <li><b>UObject overhead:</b> Each simulated actor/component in Unreal derives from the <code>UObject</code> system, which carries metadata for editor/engine features and is tracked by Unreal’s garbage collector. This adds per-object memory beyond the pure physics state.<br>
  <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-object-handling-in-unreal-engine#garbagecollection" target="_blank">Unreal Object Handling – Garbage Collection</a></li>

  <li><b>Collision/constraint data in Chaos:</b> Chaos (UE 5.7) is a general physics system that handles rigid bodies, collisions, and constraints with a full feature set. Managing contacts and constraints between many bodies increases memory usage with object count.<br>
  <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine" target="_blank">Physics in Unreal Engine (Chaos)</a> •
  <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/collision-in-unreal-engine---overview" target="_blank">Collision Overview</a></li>
</ul>

<p>
By contrast, Unity’s physics runs inside the engine’s native C/C++ core (PhysX integration) with a smaller per-object footprint exposed to C# scripts, which helps keep overall memory lower in large rigidbody scenes.<br>
<a href="https://docs.unity.cn/6000.0/Documentation/Manual/performance-native-memory.html" target="_blank">Unity Manual – Native memory (C/C++ engine core)</a> •
<a href="https://docs.unity3d.com/Manual/PhysicsSection.html" target="_blank">Unity Manual – Physics (PhysX integration)</a>
</p>

---

## Discussion

The results clearly highlight each engine’s priorities:

| Unity (PhysX) | Unreal (Chaos) |
|:--|:--|
| Lightweight, efficient physics simulation | Physically realistic, stable simulation |
| Predictable linear scaling | Non-linear scaling due to solver complexity |
| Minimal memory overhead | Large memory footprint due to detailed state tracking |
| Ideal for real-time gameplay | Ideal for cinematic or high-accuracy simulation |

Unity’s engine optimizes for **speed and scalability**, while Unreal’s Chaos aims for **physical accuracy and realism**.

---

## Conclusion

Both Unity and Unreal Engine are highly capable of simulating physics realistically and efficiently. In most cases, either engine would handle typical gameplay physics well — the choice ultimately 
depends on the project’s goals and performance requirements.

**Overall:**
- **Unity** delivers lower Frame Time, causing higher FPS and lower memory usage, ideal for interactive games and rapid simulation.  
- **Unreal** provides more accurate and stable results, but with significantly higher computational cost.  
