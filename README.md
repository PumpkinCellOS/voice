# voice

Build instructions:
```
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH=<path-to-sfml> -DSFML_STATIC_LIBRARIES=TRUE
make -j<cpu-core-count>
```

If on Windows, move `openal32.dll` to `build`.
