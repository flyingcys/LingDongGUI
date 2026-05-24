cd /Users/cys/embedded/LingDongGUI
cmake -S examples/sdl -B examples/sdl/build -DUSE_DEMO=1
cmake --build examples/sdl/build -j8
./examples/sdl/build/ldgui_sdl_demo