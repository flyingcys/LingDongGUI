#!/usr/bin/env python3
"""Validate TinyUI v2.3 widget creator / props / object-parameter contracts.

Canonical form for every public widget header under tinyui/include/widgets/:

  typedef struct tinyui_<widget>_props {
      uint32_t fields;
      uint16_t id;
      /* remaining fields with TINYUI_<WIDGET>_FIELD_* presence bits */
  } tinyui_<widget>_props_t;

  tinyui_obj_t *tinyui_<widget>_create(tinyui_obj_t *parent);
  tinyui_obj_t *tinyui_<widget>_create_with_props(
      tinyui_obj_t *parent,
      const tinyui_<widget>_props_t *props);

Exceptions:
  - window / background may use parent == NULL for root-style creators, but the
    declared parent type must still be tinyui_obj_t *.
  - scroll_selecter is the misspelled legacy name; canonical header is
    scroll_selector.h with scroll_selector symbols.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
WIDGETS_DIR = ROOT / "tinyui" / "include" / "widgets"
WIDGETS_SRC = ROOT / "tinyui" / "src" / "widgets"
RUNTIME_SRC = ROOT / "tinyui" / "src" / "core" / "runtime.c"

# Canonical widget stems expected under widgets/. background is included even
# though runtime also exposes no-arg helpers in core/runtime.h.
CANONICAL_WIDGETS = (
    "animation",
    "arc",
    "background",
    "button",
    "calendar",
    "canvas",
    "checkbox",
    "clock",
    "combo_box",
    "date_time",
    "gauge",
    "graph",
    "icon_slider",
    "image",
    "keyboard",
    "label",
    "line_edit",
    "list",
    "message_box",
    "progress_bar",
    "progress_wheel",
    "qrcode",
    "radial_menu",
    "scroll_selector",
    "slider",
    "switch",
    "table",
    "text",
    "window",
)

FORBIDDEN_LEGACY_HEADERS = ("scroll_selecter.h",)

CREATE_RE = re.compile(
    r"\btinyui_(?P<name>[A-Za-z0-9_]+)_create(?P<suffix>_with_props)?\s*\("
)
INIT_RE = re.compile(r"\btinyui_[A-Za-z0-9_]*_init\s*\(")
STRUCT_FORWARD_RE = re.compile(r"\bstruct\s+tinyui_(?P<name>[A-Za-z0-9_]+)\s*;")
PROPS_STRUCT_RE = re.compile(
    r"(?:typedef\s+)?struct\s+tinyui_(?P<name>[A-Za-z0-9_]+)_props\s*\{(?P<body>.*?)\}\s*"
    r"(?:tinyui_(?P=name)_props_t\s*)?;",
    re.DOTALL,
)
PROPS_TYPEDEF_ALIAS_RE = re.compile(
    r"typedef\s+struct\s+tinyui_(?P<name>[A-Za-z0-9_]+)_props\s+tinyui_(?P=name)_props_t\s*;"
)
FUNCTION_DECL_RE = re.compile(
    r"(?P<ret>[^;{}]+?)\b(?P<name>tinyui_[A-Za-z0-9_]+)\s*\((?P<params>[^;]*)\)\s*;",
    re.DOTALL,
)


def _strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def _normalize_space(text: str) -> str:
    return re.sub(r"\s+", " ", text).strip()


def _error(code: str, **details: object) -> dict:
    return {"code": code, **details}


def _param_list(params: str) -> list[str]:
    raw = _normalize_space(params)
    if not raw or raw == "void":
        return []
    parts: list[str] = []
    depth = 0
    start = 0
    for index, ch in enumerate(raw):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == "," and depth == 0:
            parts.append(raw[start:index].strip())
            start = index + 1
    parts.append(raw[start:].strip())
    return [p for p in parts if p]


def _is_obj_ptr(param: str, const_ok: bool = True) -> bool:
    p = _normalize_space(param)
    if const_ok and p.startswith("const "):
        p = p[6:].strip()
    return bool(re.fullmatch(r"tinyui_obj_t\s*\*\s*[A-Za-z_][A-Za-z0-9_]*", p)) or p == "tinyui_obj_t *"


def _is_props_ptr(param: str, widget: str) -> bool:
    p = _normalize_space(param)
    if p.startswith("const "):
        p = p[6:].strip()
    expected = rf"(?:const\s+)?tinyui_{widget}_props_t\s*\*\s*[A-Za-z_][A-Za-z0-9_]*"
    expected_anon = rf"(?:const\s+)?tinyui_{widget}_props_t\s*\*"
    return bool(re.fullmatch(expected, p) or re.fullmatch(expected_anon, p))


def _decl_uses_concrete_widget_type(params: str, widget: str) -> bool:
    return bool(
        re.search(rf"\bstruct\s+tinyui_{widget}\s*\*", params)
        or re.search(rf"\btinyui_{widget}_t\s*\*", params)
    )


def _check_props(text: str, widget: str) -> list[dict]:
    errors: list[dict] = []
    matches = list(PROPS_STRUCT_RE.finditer(text))
    if not matches:
        errors.append(_error("missing_props_struct", widget=widget))
        return errors

    if len(matches) > 1:
        errors.append(_error("duplicate_props_struct", widget=widget, count=len(matches)))

    match = matches[0]
    body = match.group("body")
    normalized_body = _normalize_space(body)
    if not re.match(r"uint32_t\s+fields\s*;\s*uint16_t\s+id\s*;", normalized_body):
        errors.append(
            _error(
                "props_missing_presence_mask",
                widget=widget,
                detail="props must start with uint32_t fields; uint16_t id;",
                body=normalized_body[:120],
            )
        )

    if "const char *id" in normalized_body or "const char* id" in normalized_body:
        errors.append(_error("props_legacy_string_id", widget=widget))

    # Prefer typedef alias form used by core style/runtime: either
    #   typedef struct tinyui_x_props { ... } tinyui_x_props_t;
    # or separate typedef after incomplete struct.
    has_inline_typedef = bool(
        re.search(
            rf"typedef\s+struct\s+tinyui_{widget}_props\s*\{{.*?\}}\s*tinyui_{widget}_props_t\s*;",
            text,
            re.DOTALL,
        )
    )
    has_alias = bool(PROPS_TYPEDEF_ALIAS_RE.search(text)) or has_inline_typedef
    if not has_alias and f"tinyui_{widget}_props_t" not in text:
        errors.append(_error("missing_props_typedef", widget=widget))

    field_macro_prefix = f"TINYUI_{widget.upper()}_FIELD_"
    if field_macro_prefix not in text and f"tinyui_{widget}_field" not in text.lower():
        # Allow enum tinyui_<widget>_field { TINYUI_..._FIELD_... }
        if not re.search(rf"TINYUI_{widget.upper()}_FIELD_[A-Z0-9_]+", text):
            errors.append(_error("missing_props_field_bits", widget=widget))

    return errors


def _check_creators(text: str, widget: str) -> list[dict]:
    errors: list[dict] = []
    # Root background creators live in core/runtime.h (no parent).
    if widget == "background":
        if re.search(r"\btinyui_background_create\s*\(", text):
            errors.append(_error("background_creator_belongs_in_runtime", widget=widget))
        return errors
    decls = list(FUNCTION_DECL_RE.finditer(text))
    create_decls = []
    create_props_decls = []
    for decl in decls:
        name = decl.group("name")
        if name == f"tinyui_{widget}_create":
            create_decls.append(decl)
        elif name == f"tinyui_{widget}_create_with_props":
            create_props_decls.append(decl)
        elif name.endswith("_create") or name.endswith("_create_with_props"):
            # Extra creators for this header (e.g. create_child, legacy)
            if name.startswith(f"tinyui_{widget}_"):
                errors.append(
                    _error(
                        "extra_creator",
                        widget=widget,
                        symbol=name,
                        signature=_normalize_space(
                            f"{decl.group('ret')}{name}({decl.group('params')});"
                        ),
                    )
                )

    if not create_decls:
        errors.append(_error("missing_create", widget=widget))
    elif len(create_decls) > 1:
        errors.append(_error("duplicate_create", widget=widget, count=len(create_decls)))
    else:
        decl = create_decls[0]
        ret = _normalize_space(decl.group("ret"))
        params = _param_list(decl.group("params"))
        sig = _normalize_space(f"{ret} tinyui_{widget}_create({decl.group('params')});")
        if not ret.endswith("tinyui_obj_t *") and ret != "tinyui_obj_t *":
            errors.append(
                _error(
                    "noncanonical_create_return",
                    widget=widget,
                    signature=sig,
                    expected="tinyui_obj_t *",
                )
            )
        if len(params) != 1 or not _is_obj_ptr(params[0], const_ok=False):
            errors.append(
                _error(
                    "noncanonical_create_params",
                    widget=widget,
                    signature=sig,
                    expected="tinyui_obj_t *parent",
                )
            )
        if "const char *" in decl.group("params") or "struct tinyui_window" in decl.group(
            "params"
        ) or "struct tinyui_app" in decl.group("params") or "struct tinyui_widget" in decl.group(
            "params"
        ):
            errors.append(
                _error(
                    "legacy_create_signature",
                    widget=widget,
                    signature=sig,
                )
            )

    if not create_props_decls:
        errors.append(_error("missing_create_with_props", widget=widget))
    elif len(create_props_decls) > 1:
        errors.append(
            _error(
                "duplicate_create_with_props",
                widget=widget,
                count=len(create_props_decls),
            )
        )
    else:
        decl = create_props_decls[0]
        ret = _normalize_space(decl.group("ret"))
        params = _param_list(decl.group("params"))
        sig = _normalize_space(
            f"{ret} tinyui_{widget}_create_with_props({decl.group('params')});"
        )
        if not ret.endswith("tinyui_obj_t *") and ret != "tinyui_obj_t *":
            errors.append(
                _error(
                    "noncanonical_create_with_props_return",
                    widget=widget,
                    signature=sig,
                    expected="tinyui_obj_t *",
                )
            )
        if (
            len(params) != 2
            or not _is_obj_ptr(params[0], const_ok=False)
            or not _is_props_ptr(params[1], widget)
        ):
            errors.append(
                _error(
                    "noncanonical_create_with_props_params",
                    widget=widget,
                    signature=sig,
                    expected=f"tinyui_obj_t *parent, const tinyui_{widget}_props_t *props",
                )
            )

    if INIT_RE.search(text):
        errors.append(_error("forbidden_init_creator", widget=widget))

    return errors


def _check_object_params(text: str, widget: str) -> list[dict]:
    errors: list[dict] = []
    for decl in FUNCTION_DECL_RE.finditer(text):
        name = decl.group("name")
        if not name.startswith("tinyui_"):
            continue
        if name in {
            f"tinyui_{widget}_create",
            f"tinyui_{widget}_create_with_props",
        }:
            continue
        # Skip pure type helpers without object handle if any.
        params = decl.group("params")
        if _decl_uses_concrete_widget_type(params, widget):
            errors.append(
                _error(
                    "concrete_widget_param",
                    widget=widget,
                    symbol=name,
                    signature=_normalize_space(
                        f"{decl.group('ret')}{name}({params});"
                    ),
                )
            )
        # Public headers must not take struct tinyui_window * / app * for widget APIs.
        if re.search(r"\bstruct\s+tinyui_(?:window|app)\s*\*", params):
            if name.startswith(f"tinyui_{widget}_"):
                errors.append(
                    _error(
                        "legacy_parent_type_param",
                        widget=widget,
                        symbol=name,
                        signature=_normalize_space(
                            f"{decl.group('ret')}{name}({params});"
                        ),
                    )
                )
    return errors


def _check_no_public_concrete_forward(text: str, widget: str) -> list[dict]:
    errors: list[dict] = []
    for match in STRUCT_FORWARD_RE.finditer(text):
        name = match.group("name")
        if name == widget:
            errors.append(
                _error(
                    "public_concrete_struct_forward",
                    widget=widget,
                    detail=f"struct tinyui_{widget};",
                )
            )
    return errors


def check_widget_header(path: Path) -> list[dict]:
    widget = path.stem
    text = _strip_comments(path.read_text(encoding="utf-8", errors="replace"))
    errors: list[dict] = []

    if widget == "scroll_selecter":
        errors.append(
            _error(
                "legacy_misspelled_header",
                widget=widget,
                path=str(path.relative_to(ROOT)),
                expected="widgets/scroll_selector.h",
            )
        )
        return errors

    if widget not in CANONICAL_WIDGETS:
        errors.append(
            _error(
                "unexpected_widget_header",
                widget=widget,
                path=str(path.relative_to(ROOT)),
            )
        )
        return errors

    if f'#include "core/obj.h"' not in path.read_text(encoding="utf-8", errors="replace"):
        # Allow transitive includes only if obj.h already pulled; require direct include.
        errors.append(_error("missing_obj_include", widget=widget))

    errors.extend(_check_props(text, widget))
    errors.extend(_check_creators(text, widget))
    errors.extend(_check_object_params(text, widget))
    errors.extend(_check_no_public_concrete_forward(text, widget))
    return errors


def _extract_function_body(text: str, name: str) -> str | None:
    m = re.search(rf"\b{re.escape(name)}\s*\([^{{;]*\)\s*\{{", text, re.S)
    if not m:
        return None
    open_i = text.find("{", m.start())
    if open_i < 0:
        return None
    depth = 0
    i = open_i
    n = len(text)
    in_str = in_chr = in_line = in_block = False
    while i < n:
        ch = text[i]
        nxt = text[i + 1] if i + 1 < n else ""
        if in_line:
            if ch == "\n":
                in_line = False
            i += 1
            continue
        if in_block:
            if ch == "*" and nxt == "/":
                in_block = False
                i += 2
                continue
            i += 1
            continue
        if in_str:
            if ch == "\\":
                i += 2
                continue
            if ch == '"':
                in_str = False
            i += 1
            continue
        if in_chr:
            if ch == "\\":
                i += 2
                continue
            if ch == "'":
                in_chr = False
            i += 1
            continue
        if ch == "/" and nxt == "/":
            in_line = True
            i += 2
            continue
        if ch == "/" and nxt == "*":
            in_block = True
            i += 2
            continue
        if ch == '"':
            in_str = True
            i += 1
            continue
        if ch == "'":
            in_chr = True
            i += 1
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return text[open_i + 1 : i]
        i += 1
    return None


def check_create_with_props_presence(root: Path) -> list[dict]:
    """Static scan: create_with_props must gate setters with props->fields bits."""
    errors: list[dict] = []
    src_dir = root / "tinyui" / "src" / "widgets"
    runtime_src = root / "tinyui" / "src" / "core" / "runtime.c"

    # Map public widget stem -> .c stem
    src_stem = {w: w for w in CANONICAL_WIDGETS}
    src_stem["scroll_selector"] = "scroll_selector"
    # background create_with_props is in runtime.c
    special_src = {"background": runtime_src}

    anti_patterns = [
        (
            "props_id_nonzero_gate",
            re.compile(r"props\s*->\s*id\s*!=\s*0"),
            "must not use props->id != 0 as presence/validity gate",
        ),
        (
            "props_text_nonzero_gate",
            re.compile(r"props\s*->\s*text\s*!=\s*(?:0|NULL)"),
            "must not use props->text != 0/NULL as presence gate",
        ),
        (
            "props_width_positive_gate",
            re.compile(r"props\s*->\s*width\s*>\s*0"),
            "must not use props->width > 0 as presence gate",
        ),
        (
            "props_height_positive_gate",
            re.compile(r"props\s*->\s*height\s*>\s*0"),
            "must not use props->height > 0 as presence gate",
        ),
        (
            "props_sentinel_neg1_gate",
            re.compile(r"props\s*->\s*\w+\s*!=\s*-1"),
            "must not use != -1 sentinel as presence gate",
        ),
        (
            "legacy_present_mask",
            re.compile(r"\bpresent_mask\b"),
            "must not use present_mask; unify on props->fields",
        ),
    ]

    for widget in CANONICAL_WIDGETS:
        if widget in special_src:
            path = special_src[widget]
        else:
            path = src_dir / f"{src_stem[widget]}.c"
        if not path.exists():
            errors.append(
                _error(
                    "missing_widget_source",
                    widget=widget,
                    path=str(path.relative_to(root)),
                )
            )
            continue

        text = path.read_text(encoding="utf-8", errors="replace")
        body = _extract_function_body(text, f"tinyui_{widget}_create_with_props")
        if body is None:
            errors.append(
                _error(
                    "missing_create_with_props_impl",
                    widget=widget,
                    path=str(path.relative_to(root)),
                )
            )
            continue

        # Null props may early-return to plain create; remaining path must use fields.
        if "props->fields" not in body and "props->fields" not in body.replace(" ", ""):
            # also accept (props->fields & ...)
            if not re.search(r"props\s*->\s*fields", body):
                errors.append(
                    _error(
                        "create_with_props_missing_fields_mask",
                        widget=widget,
                        path=str(path.relative_to(root)),
                        detail="create_with_props body must reference props->fields",
                    )
                )

        # Must gate with TINYUI_<W>_FIELD_ at least once when props non-null path exists.
        field_prefix = f"TINYUI_{widget.upper()}_FIELD_"
        if field_prefix not in body and "props->fields" in body.replace(" ", " "):
            # allow if only ID field and documented - still require at least one FIELD macro
            if not re.search(rf"TINYUI_{widget.upper()}_FIELD_[A-Z0-9_]+", body):
                errors.append(
                    _error(
                        "create_with_props_missing_field_macros",
                        widget=widget,
                        path=str(path.relative_to(root)),
                    )
                )
        elif not re.search(rf"TINYUI_{widget.upper()}_FIELD_[A-Z0-9_]+", body):
            errors.append(
                _error(
                    "create_with_props_missing_field_macros",
                    widget=widget,
                    path=str(path.relative_to(root)),
                )
            )

        for code, pattern, detail in anti_patterns:
            if pattern.search(body):
                errors.append(
                    _error(
                        code,
                        widget=widget,
                        path=str(path.relative_to(root)),
                        detail=detail,
                    )
                )

        # File-level: gauge/message_box props_are_valid must not require id != 0
        if re.search(
            rf"tinyui_{re.escape(widget)}_props_are_valid[\s\S]{{0,400}}props->id\s*!=\s*0",
            text,
        ):
            errors.append(
                _error(
                    "props_valid_requires_id",
                    widget=widget,
                    path=str(path.relative_to(root)),
                    detail="props_are_valid must not require props->id != 0",
                )
            )

        # qrcode header must not expose present_mask as required presence mechanism
        if widget == "qrcode":
            header = root / "tinyui" / "include" / "widgets" / "qrcode.h"
            htext = header.read_text(encoding="utf-8", errors="replace")
            if re.search(r"\bpresent_mask\b", htext):
                errors.append(
                    _error(
                        "qrcode_present_mask_in_header",
                        widget=widget,
                        path=str(header.relative_to(root)),
                        detail="remove present_mask; use props->fields only",
                    )
                )
            if "TINYUI_QRCODE_FIELD_PRESENT_MASK" in htext:
                errors.append(
                    _error(
                        "qrcode_present_mask_field_bit",
                        widget=widget,
                        path=str(header.relative_to(root)),
                    )
                )

        # Forbid silent (void)props->member except props->id (FIELD_ID bookkeeping).
        for member in re.findall(r"\(void\)\s*props\s*->\s*(\w+)", body):
            if member == "id":
                continue
            errors.append(
                _error(
                    "create_with_props_void_props_member",
                    widget=widget,
                    path=str(path.relative_to(root)),
                    detail=f"(void)props->{member} is forbidden; call a setter or fail-closed",
                )
            )

        # Multi-arg composite: missing sibling fields must not hardcode zeros / epoch.
        composite_hardcodes = [
            (
                "composite_hardcoded_date_epoch",
                re.compile(r"props\s*->\s*year\s*:\s*1970|:\s*1970\b"),
                "partial date must read current get_date, not 1970",
            ),
            (
                "composite_hardcoded_anchor_half",
                re.compile(r"props\s*->\s*\w*anchor_\w+\s*:\s*0\.5f|:\s*0\.5f\b"),
                "partial clock anchor must read host anchor, not 0.5f",
            ),
            (
                "composite_hardcoded_radial_axis",
                re.compile(
                    r"props\s*->\s*x_axis\s*:\s*0|props\s*->\s*y_axis\s*:\s*0|"
                    r"props\s*->\s*item_max\s*:\s*1\b"
                ),
                "partial radial geometry must read host x_axis/y_axis/item_max",
            ),
            (
                "composite_hardcoded_icon_layout",
                re.compile(
                    r"props\s*->\s*icon_space\s*:\s*0|props\s*->\s*pages\s*:\s*1\b|"
                    r"props\s*->\s*icon_width\s*:\s*48|props\s*->\s*columns\s*:\s*4|"
                    r"props\s*->\s*rows\s*:\s*1\b"
                ),
                "partial icon_slider layout must read host layout fields",
            ),
            (
                "composite_hardcoded_centre_offset",
                re.compile(
                    r"props\s*->\s*centre_offset_x\s*:\s*0|"
                    r"props\s*->\s*centre_offset_y\s*:\s*0"
                ),
                "partial gauge centre_offset must read host offsets",
            ),
            (
                "composite_hardcoded_time_zero",
                re.compile(
                    r"props\s*->\s*hour\s*:\s*0|props\s*->\s*minute\s*:\s*0|"
                    r"props\s*->\s*second\s*:\s*0"
                ),
                "partial time must read current get_time, not 0:0:0",
            ),
            (
                "composite_hardcoded_month_day",
                re.compile(r"props\s*->\s*month\s*:\s*1\b|props\s*->\s*day\s*:\s*1\b"),
                "partial date month/day must read current get_date",
            ),
        ]
        for code, pattern, detail in composite_hardcodes:
            if pattern.search(body):
                errors.append(
                    _error(
                        code,
                        widget=widget,
                        path=str(path.relative_to(root)),
                        detail=detail,
                    )
                )

        # Half-size: set_size must not force missing dim to 0 via ternary.
        if re.search(
            r"tinyui_widget_set_size\s*\([^;]*\?\s*props->width\s*:\s*0",
            body,
        ) or re.search(
            r"tinyui_widget_set_size\s*\([^;]*\?\s*props->height\s*:\s*0",
            body,
        ):
            errors.append(
                _error(
                    "create_with_props_half_size_zero",
                    widget=widget,
                    path=str(path.relative_to(root)),
                    detail="missing WIDTH/HEIGHT must keep current size, not force 0",
                )
            )

        # table/graph create dims must enter create path, not void after 1x1 create
        if widget == "table" and re.search(
            r"TINYUI_TABLE_FIELD_(?:ROWS|COLUMNS)", body
        ):
            if re.search(r"\(void\)\s*props\s*->\s*(?:rows|columns)", body):
                errors.append(
                    _error(
                        "table_rows_columns_not_applied",
                        widget=widget,
                        path=str(path.relative_to(root)),
                    )
                )
        if widget == "graph" and "TINYUI_GRAPH_FIELD_SERIES_MAX" in body:
            if re.search(r"\(void\)\s*props\s*->\s*series_max", body):
                errors.append(
                    _error(
                        "graph_series_max_not_applied",
                        widget=widget,
                        path=str(path.relative_to(root)),
                    )
                )

    # Cross-cutting: as_* kind-check failure must not return 0 on int setters.
    for c_path in sorted(src_dir.glob("*.c")):
        c_text = c_path.read_text(encoding="utf-8", errors="replace")
        # line_edit must kind-check
        if c_path.name == "line_edit.c":
            as_body = _extract_function_body(c_text, "tinyui_line_edit_as_line_edit")
            if as_body is not None and (
                "tinyui_widget_is_kind" not in as_body
                and "tinyui_runtime_internal_widget_is_kind" not in as_body
            ):
                errors.append(
                    _error(
                        "line_edit_as_missing_kind_check",
                        widget="line_edit",
                        path=str(c_path.relative_to(root)),
                    )
                )
        # int tinyui_X_set_... with as_ then return 0
        for m in re.finditer(
            r"^(?P<ret>int)\s+(?P<name>tinyui_\w+)\s*\([^;]*\)\s*\{(?P<body>.*?)(?=^int\s+tinyui_|^tinyui_obj_t\s*\*|^void\s+tinyui_|\Z)",
            c_text,
            re.M | re.S,
        ):
            name = m.group("name")
            body = m.group("body")
            if "_create" in name or name.endswith("_are_valid"):
                continue
            if re.search(r"_as_\w+\s*\(", body) and re.search(
                r"if\s*\(\s*\w+\s*==\s*0\s*\)\s*\{\s*return\s+0\s*;\s*\}", body
            ):
                # first early return 0 after as_ is the fake-success pattern
                as_i = body.find("_as_")
                ret0 = re.search(r"if\s*\(\s*\w+\s*==\s*0\s*\)\s*\{\s*return\s+0\s*;\s*\}", body)
                if ret0 and as_i >= 0 and ret0.start() > as_i and ret0.start() - as_i < 200:
                    errors.append(
                        _error(
                            "setter_kind_fail_returns_zero",
                            widget=c_path.stem,
                            symbol=name,
                            path=str(c_path.relative_to(root)),
                            detail="kind/obj fail must return -1, not 0",
                        )
                    )
                    # one per function is enough; continue to next fn
                    continue

    return errors


def check_contract(root: Path | None = None) -> list[dict]:
    root = root or ROOT
    widgets_dir = root / "tinyui" / "include" / "widgets"
    errors: list[dict] = []

    if not widgets_dir.is_dir():
        return [_error("missing_widgets_dir", path=str(widgets_dir))]

    headers = sorted(widgets_dir.glob("*.h"))
    present = {h.stem for h in headers}

    for forbidden in FORBIDDEN_LEGACY_HEADERS:
        if (widgets_dir / forbidden).exists():
            errors.append(
                _error(
                    "legacy_misspelled_header",
                    widget=Path(forbidden).stem,
                    path=f"tinyui/include/widgets/{forbidden}",
                    expected="widgets/scroll_selector.h",
                )
            )

    for widget in CANONICAL_WIDGETS:
        header = widgets_dir / f"{widget}.h"
        if not header.exists():
            errors.append(
                _error(
                    "missing_widget_header",
                    widget=widget,
                    path=str(header.relative_to(root)),
                )
            )
            continue
        errors.extend(check_widget_header(header))

    # Flag unexpected extras besides known temporary names already handled.
    for stem in sorted(present - set(CANONICAL_WIDGETS) - {"scroll_selecter"}):
        errors.append(
            _error(
                "unexpected_widget_header",
                widget=stem,
                path=f"tinyui/include/widgets/{stem}.h",
            )
        )

    errors.extend(check_create_with_props_presence(root))
    return errors


def _format_error(err: dict) -> str:
    code = err.get("code", "error")
    widget = err.get("widget", "?")
    parts = [f"[{code}] {widget}"]
    if "symbol" in err:
        parts.append(f"symbol={err['symbol']}")
    if "signature" in err:
        parts.append(f"sig={err['signature']}")
    if "expected" in err:
        parts.append(f"expected={err['expected']}")
    if "detail" in err:
        parts.append(str(err["detail"]))
    if "path" in err:
        parts.append(f"path={err['path']}")
    if "body" in err:
        parts.append(f"body={err['body']}")
    return " | ".join(parts)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--root",
        type=Path,
        default=ROOT,
        help="repository root (default: inferred)",
    )
    args = parser.parse_args(argv)

    errors = check_contract(args.root.resolve())
    if not errors:
        print("tinyui widget creator contract: OK")
        print(f"widgets checked: {len(CANONICAL_WIDGETS)}")
        return 0

    print(f"tinyui widget creator contract: FAIL ({len(errors)} issue(s))", file=sys.stderr)
    # Group by widget for readable RED output.
    by_widget: dict[str, list[dict]] = {}
    for err in errors:
        by_widget.setdefault(str(err.get("widget", "?")), []).append(err)
    for widget in sorted(by_widget):
        print(f"\n## {widget}", file=sys.stderr)
        for err in by_widget[widget]:
            print(f"  - {_format_error(err)}", file=sys.stderr)

    # Compact summary of non-canonical creators for TDD red evidence.
    creator_issues = [
        e
        for e in errors
        if e.get("code", "").startswith(
            (
                "noncanonical_",
                "legacy_",
                "missing_create",
                "extra_creator",
                "forbidden_init",
                "props_",
                "missing_props",
                "concrete_widget",
                "public_concrete",
            )
        )
        or e.get("code")
        in {
            "missing_create_with_props",
            "duplicate_create",
            "duplicate_create_with_props",
            "missing_props_field_bits",
            "missing_obj_include",
            "legacy_misspelled_header",
            "missing_widget_header",
            "create_with_props_missing_fields_mask",
            "create_with_props_missing_field_macros",
            "props_id_nonzero_gate",
            "props_text_nonzero_gate",
            "props_width_positive_gate",
            "props_height_positive_gate",
            "props_sentinel_neg1_gate",
            "legacy_present_mask",
            "props_valid_requires_id",
            "qrcode_present_mask_in_header",
            "qrcode_present_mask_field_bit",
            "missing_create_with_props_impl",
            "missing_widget_source",
            "create_with_props_void_props_member",
            "create_with_props_half_size_zero",
            "table_rows_columns_not_applied",
            "graph_series_max_not_applied",
            "line_edit_as_missing_kind_check",
            "setter_kind_fail_returns_zero",
            "composite_hardcoded_date_epoch",
            "composite_hardcoded_anchor_half",
            "composite_hardcoded_radial_axis",
            "composite_hardcoded_icon_layout",
            "composite_hardcoded_centre_offset",
            "composite_hardcoded_time_zero",
            "composite_hardcoded_month_day",
        }
    ]
    print(
        f"\nnon-canonical creator/props issues: {len(creator_issues)}",
        file=sys.stderr,
    )
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
