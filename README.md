# PlatinumSrc
A WIP 90's style 3D engine<br>

---
<!-- The DLLs needed for Windows are stored in the [`windll`](https://github.com/PQCraft/PlatinumSrc/tree/windll) branch<br> -->
Compiling on Windows has not been tested and is most likely broken. The dependencies have also changed so the dlls in the `windll` branch are most likely no longer valid.

---
Progress (checked = done, <- = being worked on):<br>
- [ ] Sound engine
  - [X] Music
  - [ ] Sound effects
- [ ] UI engine <-
  - [X] Text rendering
  - [X] Box rendering
  - [X] Element rendering
  - [X] Collision and click detection
    [ ] More elements **<-**
    [ ] Text input
- [ ] Graphics engine
  - [ ] In-game HUD rendering
  - [X] Objects
  - [X] Object render stack
  - [X] 2D
    - [X] Shapes
    - [X] Coloring
    - [X] Texturing
  - [X] 3D
    - [X] Lighting
    - [X] Shading
    - [X] Object loading
  - [ ] Loading alternate renderers from .so/.dll files
- [ ] Game engine
  - [ ] Map loading
  - [ ] Physics
  - [ ] Loading games from .so/.dll files

