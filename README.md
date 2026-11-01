# simple-c-01

A native macOS C17 starter using Apple Clang, CMake, Ninja and CTest.

## Build, run and test

```sh
cd ~/Projects/simple-c-01
cmake --preset debug
cmake --build --preset debug
./build/debug/simple_c_01
ctest --preset debug
```

Configure generates build files and the compilation database. Build compiles the program. Run the build command after editing source; rerun configure after changing CMake settings or adding source files to the build. Generated files stay in `build/debug/`.

Expected program output: `Hello from C! 20 + 22 = 42`.

## Release (shipping build)

Use the Release preset to compile an optimized native executable:

```sh
cd ~/Projects/simple-c-01
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/release/simple_c_01
```

The executable to distribute is `build/release/simple_c_01`. Release and Debug use separate build directories, so both can coexist. After editing code, repeat the build and test commands; configure is needed initially and after changes to CMake settings.

Release uses CMake's compiler-specific optimization flags and defines `NDEBUG`, which disables standard C `assert()` checks. Tests remain enabled so you can test the optimized code before shipping; the test executable does not need to be distributed. Use Debug for stepping through code and inspecting variables, since optimization can rearrange code or remove variables.

This is a native build for the current Mac's architecture, not a universal macOS binary or an installer. Signing, notarization and packaging are separate distribution steps if your delivery method requires them. The existing LazyVim debugger and `.clangd` configuration still use `build/debug/`; build Debug when using those workflows.

## Project layout

- `src/main.c`: program entry point; start here.
- `src/math_utils.c`: a reusable function implementation.
- `include/math_utils.h`: its public declaration and documentation.
- `tests/test_math_utils.c`: checks positive, negative and zero inputs through CTest.
- `CMakeLists.txt`: sources, include paths, warning options and test registration.
- `CMakePresets.json`: reproducible Debug and Release configurations and build/test presets.
- `.clangd`: tells clangd where the compilation database lives.
- `.clang-format`: formatting style, applied by your LazyVim setup on save.
- `.vscode/launch.json`: debugger configuration also read by your LazyVim DAP setup.

Add new `.c` files to the appropriate target in `CMakeLists.txt`. Keep header paths in `target_include_directories` and compile-time macros in `target_compile_definitions`.

## Open in LazyVim

```sh
cd ~/Projects/simple-c-01
nvim src/main.c
```

Space e opens the explorer, Space Space finds files, and Space sg searches text. On `add`, press K for documentation or gd to navigate to its implementation. Ctrl-o jumps back. Space cl shows attached servers; Space cf formats the current file.

The Debug configuration generates `build/debug/compile_commands.json` so clangd knows the compiler flags and the `include/` directory. If you remove the build folder, rerun configure. No global editor include-path changes are needed.

## Debug in LazyVim

Build first. In `src/main.c`, put the cursor on the `printf` line and press Space db. Press Space dc and choose **Debug simple-c-01 (native macOS)**. It starts paused; continue to reach the breakpoint.

Space du toggles debugger panes, Space de evaluates a variable, Space dO steps over, and Space dt terminates. Inspect `answer` to see 42. Rebuild after edits before debugging again. This project builds native macOS code; it is not STM32 firmware.

The CMake extra also offers `:CMakeSelectConfigurePreset`, `:CMakeGenerate`, `:CMakeBuild`, and `:CMakeDebug`. The terminal commands above are the simplest starting workflow.
