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


./build/examples/sdl/tinyui_demo animation_basic
./build/examples/sdl/tinyui_demo arc_basic
./build/examples/sdl/tinyui_demo basic_widgets
./build/examples/sdl/tinyui_demo calendar_basic
./build/examples/sdl/tinyui_demo clock_basic
./build/examples/sdl/tinyui_demo combo_box_basic
./build/examples/sdl/tinyui_demo date_time_basic
./build/examples/sdl/tinyui_demo gauge_basic
./build/examples/sdl/tinyui_demo graph_basic
./build/examples/sdl/tinyui_demo grid_parity
./build/examples/sdl/tinyui_demo hello_world
./build/examples/sdl/tinyui_demo icon_slider_basic
./build/examples/sdl/tinyui_demo keyboard_basic
./build/examples/sdl/tinyui_demo layout_flex
./build/examples/sdl/tinyui_demo layout_grid
./build/examples/sdl/tinyui_demo layout_parity
./build/examples/sdl/tinyui_demo legacy_widget_parity
./build/examples/sdl/tinyui_demo line_edit_basic
./build/examples/sdl/tinyui_demo list_basic
./build/examples/sdl/tinyui_demo message_box_basic
./build/examples/sdl/tinyui_demo progress_bar_basic
./build/examples/sdl/tinyui_demo progress_wheel_basic
./build/examples/sdl/tinyui_demo qrcode_basic
./build/examples/sdl/tinyui_demo radial_menu_basic
./build/examples/sdl/tinyui_demo scroll_selecter_basic
./build/examples/sdl/tinyui_demo settings_panel
./build/examples/sdl/tinyui_demo table_basic
./build/examples/sdl/tinyui_demo theme_showcase


./build/examples/sdl/tinyui_demo legacy_demo0_parity
./build-legacy0/ldgui_sdl_demo
