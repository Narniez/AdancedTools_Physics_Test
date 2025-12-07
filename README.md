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
| **FPS** | Frames per second (1 / ΔTime) |
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
  <img src="charts/unreal_stability.png" width="800"/>
  <br>
  <em>Figure 1 – Unreal Engine: Time until all cubes stop simulating physics</em>
</p>

<p align="center">
  <img src="charts/unity_stability.png" width="800"/>
  <br>
  <em>Figure 2 – Unity: Time until all cubes stop simulating physics</em>
</p>

#### Analysis
In **Unity**, cubes began to fall over and collapse even at 500 objects, causing the simulation to stabilize relatively quickly as energy dissipated through collisions.

In **Unreal**, due to Chaos Physics’ more realistic solver, cube towers remained upright for the **500** and **1000** cube tests.  
These standing towers stayed stable for the entire 30-second window, preventing the active object count from reaching zero.  
At **2000 cubes**, the increased pile density caused the towers to collapse, and the engine managed to sleep the cubes after some time. At **5000** and **10000** cubes
the initial towers again collapsed, but because of the high amount of cubes, the engine did not manage to stabilize after the 30-second threshold.

<h4> Visual Comparison – Cube Behavior in Unreal vs Unity</h4> <p> The following screenshots illustrate the visual difference between the two simulations. In <b>Unreal Engine</b>, the cubes form tall, stable <b>towers</b> that remain upright throughout the simulation, preventing the active object count from reaching zero. In contrast, <b>Unity</b> cubes quickly collapse into piles, losing kinetic energy faster and allowing the simulation to stabilize much earlier. </p> <table align="center"> <tr> <td align="center"> <img src="charts/unreal_500_towers.png" width="400"/><br> <em>Figure 1a – Unreal Engine (500 cubes): Stable towers remain upright.</em> </td> <td align="center"> <img src="charts/unity_500_cubes.png" width="400"/><br> <em>Figure 1b – Unity (500 cubes): Cubes collapse into a pile.</em> </td> </tr> <tr> <td align="center"> <img src="charts/unreal_1000_towers.png" width="400"/><br> <em>Figure 2a – Unreal Engine (1000 cubes): Tall towers remain stable.</em> </td> <td align="center"> <img src="charts/unity_1000_cubes.png" width="400"/><br> <em>Figure 2b – Unity (1000 cubes): Pile forms and stabilizes quickly.</em> </td> </tr> </table> <p> These visual results directly support the data shown above — where <b>Unreal’s</b> active object count failed to reach zero for 500 and 1000 cubes, while <b>Unity’s</b> simulations quickly stabilized due to simpler collision resolution. </p>

This demonstrates a key difference:
> Unreal’s Chaos Physics emphasizes **physical realism and stability**, while Unity’s PhysX favors **simplified and more easily damped interactions**.

---

### Frame Time vs Object Count

<p align="center">
  <img src="charts/unreal_frametime.png" width="800"/>
  <br>
  <em>Figure 3 – Unreal Engine: Average frame time (ms) vs object count</em>
</p>

<p align="center">
  <img src="charts/unity_frametime.png" width="800"/>
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
<p> Unity’s <b>PhysX</b> maintains <b>near-linear scaling</b> and remains performant even with <b>10,000 cubes</b>.<br> This efficiency comes primarily from its optimized <b>broad-phase collision detection system</b>, which quickly eliminates objects that cannot possibly collide before performing more expensive narrow-phase checks.<br> According to Unity’s official documentation, the physics engine uses algorithms such as <i>sweep-and-prune</i> and <i>multi-box pruning</i> to handle large numbers of rigid bodies efficiently, minimizing CPU workload per object.<br> <a href="https://docs.unity3d.com/Manual/physics-optimization-cpu-broad-phase.html" target="_blank">Unity Manual – Physics Optimization: Broad Phase</a> </p> 

<p> Unreal Engine 5.7’s <b>Chaos Physics</b> shows <b>exponential growth</b> in frame time as object count increases.<br> Chaos is a general-purpose physics system designed to simulate complex, high-fidelity interactions, including rigid bodies, collisions, constraints, and destruction.<br> As described in Epic Games’ documentation, it integrates these features into a unified solver that prioritizes <b>accuracy and flexibility</b> over raw performance.<br> This means that every simulated body participates fully in the constraint-solving and collision pipeline, which increases computation time as object counts grow.<br> <a href="https://dev.epicgames.com/documentation/en-us/unreal-engine/physics-in-unreal-engine" target="_blank">Unreal Engine Documentation – Physics in Unreal Engine</a> </p> <hr> 
<p> <b>In summary:</b><br> <b>PhysX</b> is optimized for <i>real-time gameplay performance</i> through efficient filtering and batching of physics operations.<br> <b>Chaos</b> emphasizes <i>simulation accuracy and completeness</i>, trading higher computational cost for more realistic physical behavior. </p>
---

### Memory Usage vs Object Count

<p align="center">
  <img src="charts/unreal_memory.png" width="800"/>
  <br>
  <em>Figure 5 – Unreal Engine: Average memory usage (MB) vs object count</em>
</p>

<p align="center">
  <img src="charts/unity_memory.png" width="800"/>
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
Unreal consumes **6–8× more memory** than Unity, even for small simulations.  
This is largely due to:
- Per-body UObject overhead  
- Physics island data storage  
- Reflection and garbage collection systems  

Unity’s PhysX manages rigidbodies more compactly, explaining its much smaller footprint.

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

This experiment demonstrates how engine-level physics architecture shapes performance outcomes:

- **Unity** delivers higher FPS and lower memory usage, ideal for interactive games and rapid simulation.  
- **Unreal** provides more accurate and stable results, particularly at higher object densities, but with significantly higher computational cost.  
 Unity excels in efficiency.  
 Unreal excels in realism.
