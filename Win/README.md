# Windows build

Open `alchemistra.sln` in Visual Studio 2026.

The project keeps the language standard set to C++17 and references the shared project folders with relative paths:

- `../include`
- `../src`
- `../assets`
- `../lib/win/raylib`

Build output is generated under `../build/win/<Configuration>/`.
The post-build step copies `raylib.dll` and `assets/` next to the executable.
