from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
CMAKE_TEXT = (ROOT / "cmake/LingDongGUI.cmake").read_text(encoding="utf-8")
SUPPORT_TEXT = (ROOT / "tests/support/CMakeLists.txt").read_text(encoding="utf-8")
TESTS_TEXT = (ROOT / "tests/picoui/CMakeLists.txt").read_text(encoding="utf-8")


def main() -> int:
    assert "add_library(picoui_core_v1_1 STATIC" in CMAKE_TEXT
    assert "add_library(picoui_widgets_v1_1 STATIC" in CMAKE_TEXT
    assert "add_library(picoui_port_v1_1 INTERFACE" in CMAKE_TEXT
    assert "add_library(picoui_port_sdl_v1_1 STATIC" in CMAKE_TEXT
    assert "add_library(picoui_v1_1 INTERFACE" in CMAKE_TEXT
    assert "${LD_REPO_ROOT}/picoui/port/sdl/sdl_v1_1.c" in CMAKE_TEXT
    assert "${LD_REPO_ROOT}/picoui/port/sdl/sdl.c" not in CMAKE_TEXT.split("add_library(picoui_port_sdl_v1_1 STATIC", 1)[1].split(")", 1)[0]
    assert "target_include_directories(picoui_core_v1_1\n        PUBLIC\n            ${LD_REPO_ROOT}/picoui/include\n        PRIVATE\n            ${LD_REPO_ROOT}/picoui/src/core" in CMAKE_TEXT
    assert "target_include_directories(picoui_widgets_v1_1\n        PUBLIC\n            ${LD_REPO_ROOT}/picoui/include\n        PRIVATE\n            ${LD_REPO_ROOT}/picoui/src/core" in CMAKE_TEXT
    assert "target_link_libraries(picoui_port_sdl_v1_1 PUBLIC picoui_core_v1_1)" in CMAKE_TEXT
    assert "target_link_libraries(picoui_port_sdl_v1_1 PUBLIC picoui_core)\n" not in CMAKE_TEXT
    assert "target_link_libraries(picoui_port_v1_1 INTERFACE picoui_port_sdl_v1_1)" in CMAKE_TEXT
    assert "target_link_libraries(picoui_port_v1_1 INTERFACE picoui_port_sdl)" not in CMAKE_TEXT
    assert "target_link_libraries(picoui_v1_1 INTERFACE\n        picoui_core_v1_1\n        picoui_widgets_v1_1\n        picoui_port_v1_1" in CMAKE_TEXT
    assert "target_link_libraries(picoui_v1_1 INTERFACE\n        picoui\n" not in CMAKE_TEXT
    assert "target_link_libraries(picoui_v1_1_test_support PUBLIC picoui_v1_1)" in SUPPORT_TEXT
    assert "target_link_libraries(picoui_v1_1_test_support PUBLIC picoui)\n" not in SUPPORT_TEXT
    assert "target_link_libraries(picoui_test_support PUBLIC picoui)\n" in SUPPORT_TEXT
    assert "SUPPORT_LIB picoui_v1_1_test_support" in TESTS_TEXT
    assert "MAIN_LIB picoui_v1_1" in TESTS_TEXT
    assert "test_picoui_v1_1_port_sdl" in TESTS_TEXT
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
