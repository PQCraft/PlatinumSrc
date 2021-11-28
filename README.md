# PlatinumSrc
A WIP 90's style 3D engine<br>

---
**Build dependencies:**<br>
```
sdl2
sdl2_mixer
glfw
assimp
freetype2
```
- On Linux \(and MacOS with `brew`\), these will most likely be available as packages. The package names may be different depending on your distro, for example, on Debian/Ubuntu based distros the packages may be prefixed with `lib` and/or suffixed with `-dev`, and on Arch based distros glfw is suffixed with `-x11` or `-wayland` depending on what display server you want to build for.<br>
- On Windows you will need to download the libraries \(a quick Google<!-- Schmoogle--> search of "\[library name\] Windows" can help with this\) and place the binaries into src\lib \(you may need to create this folder\), place the headers in src\include, and place any DLLs in the root directory of the source tree \(the executable will be built there and Windows will search directories for the DLLs including the application's directory and the current working directory \[[source](https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order#standard-search-order-for-desktop-applications)\], which the first is guaranteed and the second is most likely\). If you are using MinGW, most libraries will already have a layout that will allow you to drag and drop the contents of the archive right into your MinGW installation. Some libraries such as `assimp` require you to build it from the source code.

---
**Progress \(checked = done, <- = being worked on\):**<br>
- [ ] Sound engine
  - [X] Music
  - [ ] Sound effects
- [ ] UI engine <-
  - [X] Text rendering
  - [X] Box rendering
  - [X] Element rendering
  - [X] Collision and click detection
  - [ ] More elements **<-**
  - [ ] Text input
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

