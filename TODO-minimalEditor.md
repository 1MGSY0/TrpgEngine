# TODO - Minimal Editor Milestone

Goal: Minimal end-to-end editor and runtime demo
- Create project
- Build 2–3 scenes (FlowNodes)
- Load background image, 3D object, Character1, Character2 (NPC), dialogue text
- Implement a dice check on character.debate
- Run in-editor; Save/Load; Export exe
- Demo flow: Start -> NPC speaks -> Choice -> Dice roll (<= debate) -> Next node (enter ship / denied) -> End

0) Project file structure snapshot (key files)
- TRPGEngine/
  - CMakeLists.txt
  - src/
    - Core/
      - Application.cpp
      - EngineManager.cpp
      - main.cpp
    - Engine/
      - EntitySystem/
        - Entity.hpp
        - EntityManager.hpp / EntityManager.cpp
        - ComponentRegistry.hpp / ComponentRegistry.cpp
        - ComponentType.hpp
        - Components/
          - BackgroundComponent.hpp / .cpp
          - CharacterComponent.hpp / .cpp
          - ChoiceComponent.hpp / .cpp
          - DialogueComponent.hpp / .cpp
          - DiceRollComponent.hpp / .cpp
          - FlowNodeComponent.hpp / .cpp
          - ModelComponent.hpp / .cpp
          - TransformComponent.hpp / .cpp         // 3D transform
          - Transform2DComponent.hpp / .cpp       // 2D transform
          - UIButtonComponent.hpp / .cpp
          - ProjectMetaComponent.hpp / .cpp
      - RenderSystem/
        - SceneManager.hpp / .cpp (referenced)
      - GameplaySystem/
        - GameInstance.hpp / .cpp (referenced)
    - Project/
      - BuildSystem.hpp / .cpp (referenced)
      - ProjectManager.hpp / .cpp (referenced)
      - RuntimeLauncher.hpp / .cpp (referenced)
    - Resources/
      - ResourceManager.hpp / .cpp (referenced)
    - UI/
      - EditorUI.cpp
      - EditorPanels.cpp
      - EntityInspectorPanel.cpp
      - EditorMenu.cpp
      - EditorDockLayout.cpp
      - EditorStatusBar.cpp
      - ScenePanel/
        - ScenePanel.cpp
      - ImGuiUtils/
        - ImGuiUtils.cpp
      - ComponentPanel/
        - RenderBackgroundPanel.hpp
        - RenderCharacterPanel.hpp
        - RenderDialoguePanel.hpp
        - RenderChoicePanel.hpp
        - RenderDicePanel.hpp
        - RenderModelPanel.hpp
        - RenderTransform2DPanel.hpp
        - RenderTransform3DPanel.hpp
        - RenderFlowNodePanel.hpp
- Runtime/
  - CMakeLists.txt
  - data.json (expected build output)
- TRPGEngine/src/Runtime/
  - DataLoader.h / DataLoader.cpp
  - RuntimeApp.h / RuntimeApp.cpp
  - main.cpp
- README.md
- TODO-minimalEditor.md

1) Immediate removals/pruning (to reduce build friction)
- [x] UI/ImGuiUtils/ImGuiUtils.cpp — Remove unused static std::vector<std::string> s_droppedFiles.
- [x] UI/ImGuiUtils/ImGuiUtils.cpp — Remove handleOSFileDrop(...) (Windows HDROP path). We already use GLFW drop callback in EditorUI; keep only one pathway.
- [x] UI/EditorPanels.cpp — Remove large commented-out loadTextureFromFile(...) stub.
- [x] Core/EngineManager.cpp — Remove duplicate include of "Project/ProjectManager.hpp" (appears twice at top).
- [x] Runtime/main.cpp — Remove wrong argument "Runtime/Assets"; runtime should read a JSON file, e.g. "Runtime/data.json".
- [x] README.md — Fix malformed code fences and stray triple backticks.

2) Build fixes and consistency
- [x] Unify include path capitalization: replace "UI/ImGUIUtils/ImGuiUtils.hpp" with "UI/ImGuiUtils/ImGuiUtils.hpp" where used.
- [x] File dialogs: add overloads openFileDialog()/saveFileDialog() with default filters.
- [x] Ensure EditorMenu.cpp includes "UI/ImGuiUtils/ImGuiUtils.hpp" and passes filter where needed for asset imports and project dialogs.
- [x] Drag & drop: keep only GLFW drop callback and remove OS drop path from ImGuiUtils.
- [x] ScenePanel framebuffer: guard zero-size panel to avoid invalid viewport/render.
- [x] Prune unused includes and stale prototypes (removed shellapi.h and a dead texture loader prototype).
- [x] Robust ImGui font loading with fallback to default font.
- [x] ComponentPanel fixes: add <imgui.h> includes; replace std::string InputText with char buffers; add missing EntityManager include in Dialogue inspector; ensure unique IDs in loops.
- [x] Enable ImGui ImVec2 math operators (IMGUI_DEFINE_MATH_OPERATORS) in EditorPanels.cpp to fix operator+ usage in Flowchart canvas.
- [x] Replace deprecated ImDrawList::AddBezierCurve with AddBezierCubic in Flowchart canvas.
- [x] Remove legacy UI/FlowPanel/Flowchart (includes/calls) and rely on built-in canvas. Ensure build scripts exclude FlowPanel files.
- [x] Fix leftover include in UI/EditorFolderPanel.cpp and correct ImGuiUtils include path in EditorUI.hpp.

3) Minimum components and serialization (Entity System)
- [ ] Ensure these components exist, serialize, and have inspectors:
  - TransformComponent (3D)
  - Transform2DComponent
  - BackgroundComponent
  - ModelComponent (Mesh3D)
  - CharacterComponent (stats including "debate")
  - DialogueComponent
  - ChoiceComponent
  - DiceRollComponent
  - FlowNodeComponent (Start/Narrative/Dialogue/Choice/DiceCheck/End)
  - UIButtonComponent
  - ProjectMetaComponent (exists)
- [ ] ComponentTypeRegistry: register factories, default values, JSON (de)serialization, and inspector UI for the above.

4) Editor panels (to add/fix)
- [x] Entity Hierarchy panel (tree, select, context menu).
- [x] Inspector: +Add Component (from registry).
- [x] Inspector: per-component editors (centralized in EntityInspectorPanel; added Remove; wired UI/ComponentPanel with drag-drop for file paths).
- [x] Flowchart node graph (MVP linking): drag nodes, drag from output port to input port to set Next; visualize links; right-click output to clear link. Further: multiple outputs, labeled edges.
- [x] Remove legacy FlowPanel renderer (m_flowChart.render) to prevent duplicate flowchart drawing.
- [~] Scene Panel overlay for dialogue/choice/dice; simple camera. (basic Play HUD overlay added; hook to flow next)
- [x] Hierarchy: New Scene creation and Rename support (register in ProjectMeta, mark unsaved, set current flow node; initialize FlowNode with safe defaults to prevent abort()).
- [x] Start node sync: setting start from Hierarchy or Inspector updates ProjectMeta.startNode and FlowNode.isStart consistently; unsaved flag set on change.

5) Play Mode (in editor)
- [ ] Game flow runner in-editor (Dialogue, Choice, DiceCheck, Narrative, End).
- [ ] HUD overlay drawn by ScenePanel during Play.

6) Save/Load/Build/Export
- [ ] Save project (.trpgproj) with entities/components/meta and asset refs.
- [ ] Load project, rebuild scene/hierarchy/flowchart.
- [ ] Build/export Runtime/data.json and assets copy. Menu to build and run TRPGRuntime.
- [x] Runtime DataLoader: align schema with flows/edges and stats.

7) Demo content checklist (what the editor must enable)
- [ ] Create Project (Untitled default OK).
- [x] Scene 1: Background image, Mesh3D (cube), NPC portrait and debate=6. (Partially supported by components and scene creation; asset import/UI next)
- [ ] Dialogue node: "I will not let you pass".
- [ ] Choice node: "Convince" or "Run".
- [ ] DiceCheck node: roll (1-10) <= debate -> success "you can enter the ship"; else "you are denied entry".
- [ ] Narrative nodes for the outcomes; End node.
- [ ] Optional: Scene 2/3 for transitions.
- [ ] Play in editor end-to-end.
- [ ] Save project, reload, Play again.
- [ ] Build/export and run TRPGRuntime with data.json.

8) Concrete code changes (next)
- [ ] ScenePanel overlay: show current flow node state (dialogue text, choices, dice roll UI) and allow interactions in Play mode.
- [ ] Component serialization: ensure all minimum components serialize/deserialize via ComponentTypeRegistry and Project save/load.
- [ ] Build/Export: hook BuildSystem to export Runtime/data.json aligned with Runtime/DataLoader schema (copy assets).

9) Risks/notes
- Keep Windows-specific dialogs behind _WIN32 guards. Provide defaults to avoid build breaks.
- Start with primitives (cube) for Mesh3D; model import can be a stub for MVP.
- Keep UI responsive: avoid blocking dialogs during Play mode.

