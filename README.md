# sdl_rpg

A native C17 starter using Clang, CMake, Ninja and CTest.

NOTICE: you cannot use these assets commercially, only for hobby projects with credits. If you found this code useful or you love it, you can follow me on x.com/ajinkyax or Subscribe on Youtube.com/c/gridshiftdev

## Build, run and test

```sh
cd ~/Projects/sdl_rpg
cmake --preset debug
cmake --build --preset debug
./build/debug/sdl_rpg
ctest --preset debug
```

Configure generates build files and the compilation database. Build compiles the program. Run the build command after editing source; rerun configure after changing CMake settings or adding source files to the build. Generated files stay in `build/debug/`.

Expected program output: `Hello from C! 20 + 22 = 42`.

## Release (shipping build)

Use the Release preset to compile an optimized native executable:

```sh
cd ~/Projects/sdl_rpg
cmake --preset release
cmake --build --preset release
ctest --preset release
./build/release/sdl_rpg
```

