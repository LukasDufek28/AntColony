# Ant Evolution Lab

A real-time, dependency-free C++20 simulation of **50,000 evolving ants**. Ants forage using pheromones, spend energy, deliver food, die and reproduce. Successful foragers are more likely to become parents; seven traits are inherited with mutation.

Searching ants perform a correlated random walk when no useful pheromone gradient is present, disperse outward near the nest, and occasionally make a larger loop-breaking turn. Strong trails gradually override that exploration.

## Run on Windows

Download the Windows artifact from GitHub Actions, unzip it, and launch `AntEvolution.exe`. Windows SmartScreen may show an unsigned-app warning because this personal build has no paid code-signing certificate.

## Controls

| Key | Action |
|---|---|
| Space | Pause/resume |
| `+` / `-` | Double/halve simulation speed (1x–128x) |
| T | Toggle pheromone visualization |
| H | Toggle help panel |
| R | Start a new randomized world |
| F5 / F9 | Save/load `ant-world.ants` |
| Esc | Quit |

## Genetics

Each ant carries speed, sensor distance, sensor angle, turn rate, metabolism, pheromone deposit and exploration genes. Food deliveries raise fitness. When an ant dies, two parents are selected through small fitness tournaments and their averaged genes mutate. Lifespans are randomized so replacement happens continuously rather than as one synchronized mass generation. The population remains exactly 50,000.

## Build automatically (no compiler needed)

1. Create an empty GitHub repository and upload this folder.
2. Open **Actions**, select **Build Windows EXE**, then **Run workflow**.
3. Download the `AntEvolution-Windows` artifact from the finished run.

## Build locally

Install Visual Studio 2022 Build Tools with **Desktop development with C++**, then run:

```bat
cmake -S . -B build -A x64
cmake --build build --config Release
```

The executable will be at `build\Release\AntEvolution.exe`.
