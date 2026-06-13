cd /Users/cys/embedded/LingDongGUI
cmake -S examples/sdl -B examples/sdl/build -DUSE_DEMO=1
cmake --build examples/sdl/build -j8
./examples/sdl/build/ldgui_sdl_demo


## tinyui
cmake --build build --target tinyui_hello_world_demo && ./build/examples/sdl/tinyui_hello_world_demo

  如果你要跑别的 demo，把目标名换掉就行，比如：

  - tinyui_basic_widgets_demo
  - tinyui_keyboard_basic_demo
  - tinyui_calendar_basic_demo
  - tinyui_theme_showcase_demo

## switch

 有 switch 的是这两个 demo：

  - tinyui_basic_widgets_demo
  - tinyui_settings_panel_demo

  最直接建议跑 basic_widgets，它明确创建了 wifi switch，还绑了 toggle 回调：

  cmake --build build --target tinyui_basic_widgets_demo && ./build/examples/sdl/tinyui_basic_widgets_demo

  如果你想看更像设置页的场景，就跑：

  cmake --build build --target tinyui_settings_panel_demo && ./build/examples/sdl/tinyui_settings_panel_demo


   PICOUI_TOUCH_LOG=1 ./build/examples/sdl/tinyui_basic_widgets_demo > 1.log 2>&1