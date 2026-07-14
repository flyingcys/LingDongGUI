#!/usr/bin/env python3
"""Generate private runtime prototypes + v22 demo bridge for Task 7."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
APP_LEGACY = ROOT / "tinyui/include/internal/app_legacy.h"
WIDGET_LEGACY = ROOT / "tinyui/include/internal/widget_legacy.h"
INTERNAL_H = ROOT / "tinyui/src/core/internal.h"
BRIDGE_H = ROOT / "tinyui/include/internal/v22_demo_bridge.h"
BRIDGE_C = ROOT / "tinyui/src/compat/v22_demo_bridge.c"

FUNC_DECL_RE = re.compile(
    r"^(?P<ret>[A-Za-z_][A-Za-z0-9_\s\*]+?)\s+(?P<name>tinyui_[A-Za-z0-9_]+)\s*\((?P<params>[^;]*)\)\s*;\s*$",
    re.MULTILINE,
)


def to_internal(name: str) -> str:
    if name.startswith("tinyui_"):
        return "tinyui_runtime_internal_" + name[len("tinyui_") :]
    return name


def extract_func_decls(text: str) -> list[tuple[str, str, str]]:
    rows = []
    for match in FUNC_DECL_RE.finditer(text):
        ret = " ".join(match.group("ret").split())
        name = match.group("name")
        params = match.group("params").strip()
        rows.append((ret, name, params))
    return rows


def strip_func_decls(text: str) -> str:
    return FUNC_DECL_RE.sub("", text)


def main() -> None:
    app_text = APP_LEGACY.read_text(encoding="utf-8")
    widget_text = WIDGET_LEGACY.read_text(encoding="utf-8")
    app_funcs = extract_func_decls(app_text)
    widget_funcs = extract_func_decls(widget_text)

    # Keep types in legacy headers; function decls move to bridge header for old names.
    APP_LEGACY.write_text(strip_func_decls(app_text), encoding="utf-8")
    WIDGET_LEGACY.write_text(strip_func_decls(widget_text), encoding="utf-8")

    # Runtime prototypes for production (internal names)
    proto_lines = [
        "/* Auto-generated Task 7: production-side renamed legacy ABI. */",
        "#ifndef TINYUI_RUNTIME_INTERNAL_LEGACY_API_H",
        "#define TINYUI_RUNTIME_INTERNAL_LEGACY_API_H",
        "",
    ]
    for ret, name, params in app_funcs + widget_funcs:
        proto_lines.append(f"{ret} {to_internal(name)}({params});")
    # Extra production symbols that were public-ish / exported
    extras = [
        "void tinyui_runtime_internal_app_pump_timers(struct tinyui_app *app, unsigned int now_ticks);",
        "struct tinyui_app *tinyui_runtime_internal_app_current(void);",
        "void tinyui_runtime_internal_app_register_host(struct tinyui_app *app, struct tinyui_widget *w);",
        "void tinyui_runtime_internal_app_unregister_host(struct tinyui_app *app, struct tinyui_widget *w);",
        "struct tinyui_widget *tinyui_runtime_internal_app_lookup_host(const struct tinyui_app *app, uint16_t name_id);",
        "uint16_t tinyui_runtime_internal_app_alloc_name_id(struct tinyui_app *app);",
        "void tinyui_runtime_internal_app_free_name_id(struct tinyui_app *app, uint16_t id);",
        "int tinyui_runtime_internal_timer_handler(void);",
        "int tinyui_runtime_internal_button_set_press(tinyui_obj_t *button, int pressed);",
        "int tinyui_runtime_internal_button_get_press(tinyui_obj_t *button, int *pressed);",
        "int tinyui_runtime_internal_tabel_show_keyboard(tinyui_obj_t *table);",
        "struct tinyui_qrcode *tinyui_runtime_internal_q_r_code_init(struct tinyui_widget *parent, const char *id);",
        "int tinyui_runtime_internal_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text);",
        "struct tinyui_arc *tinyui_runtime_internal_arc_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_animation *tinyui_runtime_internal_animation_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_clock *tinyui_runtime_internal_clock_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_date_time *tinyui_runtime_internal_date_time_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_gauge *tinyui_runtime_internal_gauge_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_icon_slider *tinyui_runtime_internal_icon_slider_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_message_box *tinyui_runtime_internal_message_box_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_progress_wheel *tinyui_runtime_internal_progress_wheel_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_radial_menu *tinyui_runtime_internal_radial_menu_init(struct tinyui_widget *parent, const char *id);",
    ]
    proto_lines.extend(extras)
    proto_lines += ["", "#endif", ""]
    runtime_api = ROOT / "tinyui/include/internal/runtime_internal_legacy_api.h"
    runtime_api.write_text("\n".join(proto_lines) + "\n", encoding="utf-8")

    # Ensure internal.h includes runtime prototypes after legacy type headers.
    internal = INTERNAL_H.read_text(encoding="utf-8")
    if "runtime_internal_legacy_api.h" not in internal:
        needle = '#include "internal/widget_legacy.h"'
        if needle in internal:
            internal = internal.replace(
                needle,
                needle + '\n#include "internal/runtime_internal_legacy_api.h"',
            )
        else:
            internal = '#include "internal/runtime_internal_legacy_api.h"\n' + internal
        INTERNAL_H.write_text(internal, encoding="utf-8")

    # Bridge header: old names for demos/tests
    bridge_h_lines = [
        "/*",
        " * INTERNAL v2.2 demo/test migration bridge — not installed, not canonical.",
        " * Enabled only with TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE=1 (PRIVATE).",
        " * Scheduled for deletion in M4.",
        " */",
        "#ifndef TINYUI_INTERNAL_V22_DEMO_BRIDGE_H",
        "#define TINYUI_INTERNAL_V22_DEMO_BRIDGE_H",
        "",
        "#if !defined(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE) || !(TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE)",
        '#error "internal/v22_demo_bridge.h requires TINYUI_ENABLE_INTERNAL_V22_DEMO_BRIDGE=1"',
        "#endif",
        "",
        '#include "internal/app_legacy.h"',
        '#include "internal/widget_legacy.h"',
        '#include "widgets/scroll_selector.h"',
        '#include "theme/theme.h"',
        "",
        "#include <stddef.h>",
        "",
    ]
    for ret, name, params in app_funcs + widget_funcs:
        bridge_h_lines.append(f"{ret} {name}({params});")
    bridge_h_lines += [
        "",
        "/* Pump alias (canonical: tinyui_process). */",
        "int tinyui_timer_handler(void);",
        "",
        "/* Typo / duplicate aliases. */",
        "int tinyui_button_set_press(tinyui_obj_t *button, int pressed);",
        "int tinyui_button_get_press(tinyui_obj_t *button, int *pressed);",
        "int tinyui_tabel_show_keyboard(tinyui_obj_t *table);",
        "int tinyui_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text);",
        "struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id);",
        "",
        "/* Widget init aliases (tests). */",
        "struct tinyui_arc *tinyui_arc_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_animation *tinyui_animation_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_clock *tinyui_clock_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_date_time *tinyui_date_time_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_gauge *tinyui_gauge_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_icon_slider *tinyui_icon_slider_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_message_box *tinyui_message_box_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_progress_wheel *tinyui_progress_wheel_init(struct tinyui_widget *parent, const char *id);",
        "struct tinyui_radial_menu *tinyui_radial_menu_init(struct tinyui_widget *parent, const char *id);",
        "",
        "/* Scroll selecter spelling aliases. */",
        "#define tinyui_scroll_selecter tinyui_scroll_selector",
        "#define tinyui_scroll_selecter_props tinyui_scroll_selector_props",
        "#define tinyui_scroll_selecter_props_t tinyui_scroll_selector_props_t",
        "#define tinyui_scroll_selecter_create tinyui_scroll_selector_create",
        "#define tinyui_scroll_selecter_create_with_props tinyui_scroll_selector_create_with_props",
        "#define tinyui_scroll_selecter_set_items tinyui_scroll_selector_set_items",
        "#define tinyui_scroll_selecter_add_item tinyui_scroll_selector_add_item",
        "#define tinyui_scroll_selecter_set_select_item_num tinyui_scroll_selector_set_select_item_num",
        "#define tinyui_scroll_selecter_set_selected_index tinyui_scroll_selector_set_selected_index",
        "#define tinyui_scroll_selecter_get_select_item_num tinyui_scroll_selector_get_select_item_num",
        "#define tinyui_scroll_selecter_get_selected_index tinyui_scroll_selector_get_selected_index",
        "#define tinyui_scroll_selecter_get_select_text tinyui_scroll_selector_get_select_text",
        "#define tinyui_scroll_selecter_set_text_color tinyui_scroll_selector_set_text_color",
        "#define tinyui_scroll_selecter_set_background_color tinyui_scroll_selector_set_background_color",
        "#define tinyui_scroll_selecter_set_bg_color tinyui_scroll_selector_set_bg_color",
        "#define tinyui_scroll_selecter_set_indicator_color tinyui_scroll_selector_set_indicator_color",
        "",
        "/* Legacy theme object helpers used by unit tests (caller-owned theme). */",
        "struct tinyui_theme *tinyui_theme_create(void);",
        "void tinyui_theme_destroy(struct tinyui_theme *theme);",
        "int tinyui_theme_apply_to_widget(struct tinyui_theme *theme, struct tinyui_widget *widget);",
        "int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme);",
        "int tinyui_theme_set_color(struct tinyui_theme *theme, int color_id, unsigned int rgb);",
        "int tinyui_theme_set_metric(struct tinyui_theme *theme, int metric_id, int value);",
        "",
        "#endif /* TINYUI_INTERNAL_V22_DEMO_BRIDGE_H */",
        "",
    ]
    BRIDGE_H.write_text("\n".join(bridge_h_lines), encoding="utf-8")

    # Bridge implementation: thin forwarders
    c_lines = [
        "/*",
        " * INTERNAL v2.2 demo/test migration bridge implementation.",
        " * Forwards to renamed production symbols / canonical APIs. M4 deletes this TU.",
        " */",
        '#include "internal/v22_demo_bridge.h"',
        "",
        '#include "internal/runtime_internal_legacy_api.h"',
        '#include "core/runtime.h"',
        '#include "theme/theme.h"',
        "",
        "#include <stdlib.h>",
        "#include <string.h>",
        "",
    ]

    def emit_forward(ret: str, name: str, params: str) -> None:
        internal = to_internal(name)
        # parameter names: keep as-is for call
        args = []
        if params.strip() and params.strip() != "void":
            for part in params.split(","):
                part = part.strip()
                # take last identifier as arg name
                m = re.search(r"([A-Za-z_][A-Za-z0-9_]*)\s*$", part.replace("[]", ""))
                if m and m.group(1) not in {
                    "int",
                    "void",
                    "char",
                    "unsigned",
                    "const",
                    "struct",
                    "enum",
                    "float",
                    "double",
                    "size_t",
                    "uint16_t",
                    "uint32_t",
                    "int16_t",
                    "int32_t",
                    "bool",
                }:
                    args.append(m.group(1))
                else:
                    args.append("/*arg*/")
        arglist = ", ".join(a for a in args if a != "/*arg*/")
        # fallback: if parsing failed, use empty for void-like
        if "/*arg*/" in args:
            # try simpler: use names from params tokens ending identifiers
            arglist = ", ".join(
                re.findall(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?\s*$", p.strip())
                for p in params.split(",")
                if p.strip() and p.strip() != "void"
            )
            arglist = ", ".join(
                x[0] if isinstance(x, tuple) else x
                for x in [
                    re.findall(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:\[[^\]]*\])?\s*$", p.strip())
                    for p in params.split(",")
                    if p.strip() and p.strip() != "void"
                ]
                if x
            )
        if ret.strip() == "void":
            c_lines.append(f"void {name}({params})")
            c_lines.append("{")
            c_lines.append(f"    {internal}({arglist});")
            c_lines.append("}")
        else:
            c_lines.append(f"{ret} {name}({params})")
            c_lines.append("{")
            c_lines.append(f"    return {internal}({arglist});")
            c_lines.append("}")
        c_lines.append("")

    for ret, name, params in app_funcs + widget_funcs:
        emit_forward(ret, name, params)

    # Special forwards
    c_lines += [
        "int tinyui_timer_handler(void)",
        "{",
        "    return tinyui_runtime_internal_timer_handler();",
        "}",
        "",
        "int tinyui_button_set_press(tinyui_obj_t *button, int pressed)",
        "{",
        "    return tinyui_runtime_internal_button_set_press(button, pressed);",
        "}",
        "",
        "int tinyui_button_get_press(tinyui_obj_t *button, int *pressed)",
        "{",
        "    return tinyui_runtime_internal_button_get_press(button, pressed);",
        "}",
        "",
        "int tinyui_tabel_show_keyboard(tinyui_obj_t *table)",
        "{",
        "    return tinyui_runtime_internal_tabel_show_keyboard(table);",
        "}",
        "",
        "int tinyui_q_r_code_set_text(tinyui_obj_t *qrcode, const char *text)",
        "{",
        "    return tinyui_runtime_internal_q_r_code_set_text(qrcode, text);",
        "}",
        "",
        "struct tinyui_qrcode *tinyui_q_r_code_init(struct tinyui_widget *parent, const char *id)",
        "{",
        "    return tinyui_runtime_internal_q_r_code_init(parent, id);",
        "}",
        "",
    ]
    for stem in (
        "arc",
        "animation",
        "clock",
        "date_time",
        "gauge",
        "icon_slider",
        "message_box",
        "progress_wheel",
        "radial_menu",
    ):
        typ = f"struct tinyui_{stem}"
        c_lines += [
            f"{typ} *tinyui_{stem}_init(struct tinyui_widget *parent, const char *id)",
            "{",
            f"    return tinyui_runtime_internal_{stem}_init(parent, id);",
            "}",
            "",
        ]

    # Theme helpers: real caller-owned theme object using public theme struct.
    c_lines += [
        "struct tinyui_theme *tinyui_theme_create(void)",
        "{",
        "    struct tinyui_theme *theme = (struct tinyui_theme *)calloc(1, sizeof(*theme));",
        "    return theme;",
        "}",
        "",
        "void tinyui_theme_destroy(struct tinyui_theme *theme)",
        "{",
        "    free(theme);",
        "}",
        "",
        "int tinyui_theme_apply_to_widget(struct tinyui_theme *theme, struct tinyui_widget *widget)",
        "{",
        "    if (theme == NULL || widget == NULL) {",
        "        return -1;",
        "    }",
        "    if (tinyui_theme_set(theme) != TINYUI_OK) {",
        "        return -1;",
        "    }",
        "    return tinyui_theme_apply((tinyui_obj_t *)widget) == TINYUI_OK ? 0 : -1;",
        "}",
        "",
        "int tinyui_app_set_theme(struct tinyui_app *app, struct tinyui_theme *theme)",
        "{",
        "    if (app == NULL || theme == NULL) {",
        "        return -1;",
        "    }",
        "    (void)app;",
        "    return tinyui_theme_set(theme) == TINYUI_OK ? 0 : -1;",
        "}",
        "",
        "int tinyui_theme_set_color(struct tinyui_theme *theme, int color_id, unsigned int rgb)",
        "{",
        "    if (theme == NULL || color_id < 0 || color_id >= (int)TINYUI_COLOR_COUNT) {",
        "        return -1;",
        "    }",
        "    theme->colors[color_id] = rgb;",
        "    return 0;",
        "}",
        "",
        "int tinyui_theme_set_metric(struct tinyui_theme *theme, int metric_id, int value)",
        "{",
        "    if (theme == NULL || metric_id < 0 || metric_id >= (int)TINYUI_METRIC_COUNT) {",
        "        return -1;",
        "    }",
        "    theme->metrics[metric_id] = (int16_t)value;",
        "    return 0;",
        "}",
        "",
    ]

    BRIDGE_C.parent.mkdir(parents=True, exist_ok=True)
    BRIDGE_C.write_text("\n".join(c_lines), encoding="utf-8")
    print(f"app_funcs={len(app_funcs)} widget_funcs={len(widget_funcs)}")
    print(f"wrote {runtime_api.relative_to(ROOT)}")
    print(f"wrote {BRIDGE_H.relative_to(ROOT)}")
    print(f"wrote {BRIDGE_C.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
