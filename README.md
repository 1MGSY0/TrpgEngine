# TRPG Game Engine

A GM-light game engine designed for immersive tabletop RPGs, featuring audio, graphics, and interactive 3D environments.

This engine supports:
- Custom interface themes
- Physics and scripting
- Random event triggers
- Hidden player-specific goals
- Built-in tools like NPC managers, dialogue boxes, and a pop-up flowchart system to visualize branching events and multiple endings

---

## What is a TRPG?

A **Tabletop Role-Playing Game (TRPG)** is a collaborative storytelling game where players take on roles in a fictional setting. A Game Master (GM) drives the narrative, and outcomes are typically determined using dice, character stats, and rule systems.

---

## Project Structure

TrpgEngine-gameloop/
│
├── cmake_run.ps1 # PowerShell script to configure & build with CMake
├── CMakeLists.txt # Root CMake configuration
├── Runtime/ # Assets (fonts, images, audio)
├── TRPGEngine/ # Core engine source code
│ ├── src/
│ │ ├── Core/ # Application logic and engine management
│ │ ├── Engine/ # Engine components and entity system
│ │ └── UI/ # (If present) UI systems
└── ...


---

## Prerequisites

### Required Tools

- [Visual Studio Code](https://code.visualstudio.com/)
- [CMake](https://cmake.org/download/)
- [PowerShell (5.0+)](https://docs.microsoft.com/en-us/powershell/)
- [Visual Studio Build Tools 2022](https://visualstudio.microsoft.com/visual-cpp-build-tools/) with:
  - C++ CMake tools for Windows
  - MSVC v143 - VS 2022 C++ x64/x86 build tools

### Recommended VS Code Extensions

Install the following from the VS Code Marketplace:
- `CMake Tools`
- `CMake`
- `C++` (`ms-vscode.cpptools`)
- `PowerShell` (`ms-vscode.powershell`)

---

## Setup Guide (Windows)

### 1. Clone or Download the Repository

If you haven't cloned from GitHub, unzip the source archive into a known directory.

```bash
# If using Git
git clone https://github.com/your-username/TrpgEngine-gameloop.git


### 2. Open in Visual Studio Code

```bash
cd TrpgEngine-gameloop
code .
```

VS Code should automatically detect `CMakeLists.txt`.

---

## Building and Running the Project

### Option 1: Using PowerShell Script (Recommended)

1. Open PowerShell in the root directory.
2. Run:

```powershell
.\cmake_run.ps1 -BuildType Release
```

Or for a debug build:

```powershell
.\cmake_run.ps1 -BuildType Debug
```

This will:
- Clean the previous build
- Configure the project for VS 2022 (x64)
- Build the solution in the specified configuration
- Run the project

### Option 2: Using CMake Tools in VS Code

1. Press `Ctrl+Shift+P` → Select `CMake: Configure`
2. Select `Visual Studio 17 2022` generator
3. Choose build type: `Debug` or `Release`
4. Press `Ctrl+Shift+P` → `CMake: Build`
5. Running the Project

After a successful build:

- Locate the built `.exe` in the `build/` folder.
- Run directly or via a debugger in VS Code:
  - Go to Run > Add Configuration... > C++ (Windows)
  - Modify `launch.json` to point to the `.exe` file

---