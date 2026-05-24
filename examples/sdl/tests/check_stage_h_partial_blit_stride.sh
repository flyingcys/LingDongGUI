#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
SRC_FILE="${ROOT_DIR}/user/Virtual_TFT_Port.c"

if [[ ! -f "${SRC_FILE}" ]]; then
    echo "FAIL: 缺少源码 ${SRC_FILE}" >&2
    exit 2
fi

python3 - "${SRC_FILE}" <<'PY'
import pathlib
import re
import sys

source = pathlib.Path(sys.argv[1]).read_text(encoding="utf-8")
errors = []

helper_match = re.search(
    r"static\s+void\s+VT_Fill_Multiple_Colors_WithStride\s*\((?P<sig>.*?)\)\s*\{(?P<body>.*?)\n\}",
    source,
    re.S,
)
if not helper_match:
    errors.append("缺少 stride-aware blit helper: VT_Fill_Multiple_Colors_WithStride")
else:
    helper_sig = helper_match.group("sig")
    helper_body = helper_match.group("body")
    if "nSourceStride" not in helper_sig:
        errors.append("stride helper 签名缺少 nSourceStride")
    for token in [
        "nSourceStride",
        "sourceRow",
        "act_x1 - x1",
        "act_y1 - y1",
    ]:
        if token not in helper_body:
            errors.append(f"stride helper 缺少关键跨行/裁剪处理: {token}")

refresh_match = re.search(
    r"bool\s+VT_sdl_refresh_task\(void\)(?P<body>.*?return\s+!sdl_joined;.*?\n\})",
    source,
    re.S,
)
if not refresh_match:
    errors.append("未找到 VT_sdl_refresh_task")
else:
    refresh_body = refresh_match.group("body")
    if "VT_Fill_Multiple_Colors_WithStride(" not in refresh_body:
        errors.append("refresh task 仍未使用 stride-aware blit helper")
    if "VT_WIDTH" not in refresh_body:
        errors.append("refresh task 未向 stride-aware helper 传入全屏 framebuffer stride")

packed_match = re.search(
    r"^void\s+VT_Fill_Multiple_Colors\s*\((?P<sig>.*?)\)\s*\{(?P<body>.*?)\n\}",
    source,
    re.S | re.M,
)
if not packed_match:
    errors.append("未找到 VT_Fill_Multiple_Colors")
else:
    packed_body = packed_match.group("body")
    if "VT_Fill_Multiple_Colors_WithStride(x1" not in packed_body:
        errors.append("packed bitmap 路径未复用 stride-aware helper")

if errors:
    print("FAIL: 阶段 H partial blit stride 修复未满足")
    for item in errors:
        print(f" - {item}")
    sys.exit(1)

print("PASS: 检测到 stride-aware partial blit 修复")
PY
