#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
SOURCE_FILE="${ROOT_DIR}/user/Virtual_TFT_Port.c"

if [[ ! -f "${SOURCE_FILE}" ]]; then
    echo "FAIL: 缺少源码 ${SOURCE_FILE}" >&2
    exit 2
fi

python3 - "${SOURCE_FILE}" <<'PY'
import pathlib
import re
import sys

source_path = pathlib.Path(sys.argv[1])
source = source_path.read_text(encoding="utf-8")

errors = []

motion_match = re.search(
    r"case SDL_MOUSEMOTION:(?P<body>.*?)(?:case SDL_WINDOWEVENT:)",
    source,
    re.S,
)
if not motion_match:
    errors.append("未找到 SDL_MOUSEMOTION 分支")
else:
    motion_body = motion_match.group("body")
    if re.search(r"\blast_x\s*=", motion_body) or re.search(r"\blast_y\s*=", motion_body):
        errors.append("SDL_MOUSEMOTION 仍直接写 committed 坐标 last_x/last_y")

pending_tokens = [
    "s_tPendingPointer",
    "s_bPointerPending",
    "s_nLastPointerCommitTick",
    "VT_sdl_resolve_motion_button_locked",
]
for token in pending_tokens:
    if token not in source:
        errors.append(f"缺少阶段 C 状态字段: {token}")

if "VT_sdl_commit_pending_pointer" not in source:
    errors.append("缺少按刷新周期提交 pending 输入的入口")

refresh_match = re.search(
    r"bool VT_sdl_refresh_task\(void\)(?P<body>.*?return !sdl_joined;.*?\n})",
    source,
    re.S,
)
if not refresh_match:
    errors.append("未找到 VT_sdl_refresh_task")
else:
    refresh_body = refresh_match.group("body")
    if "VT_sdl_commit_pending_pointer(bNeedRedraw);" not in refresh_body:
        errors.append("refresh 周期未使用 redraw 门控提交 pending pointer")

helper_match = re.search(
    r"static bool VT_sdl_should_commit_pending_pointer_locked\(Uint32 nNow, bool bForce\)(?P<body>.*?\n})",
    source,
    re.S,
)
if not helper_match:
    errors.append("未找到 pending pointer 提交门控逻辑")
else:
    helper_body = helper_match.group("body")
    for token in [
        "VT_POINTER_DRAG_COMMIT_MIN_MS",
        "VT_POINTER_DRAG_COMMIT_MIN_DELTA",
        "SDL_TICKS_PASSED",
        "s_tPendingPointer.bButtonChanged",
    ]:
        if token not in helper_body:
            errors.append(f"pending pointer 提交门控缺少: {token}")

motion_helper_match = re.search(
    r"static bool VT_sdl_resolve_motion_button_locked\(const SDL_MouseMotionEvent \*ptMotion\)(?P<body>.*?\n})",
    source,
    re.S,
)
if not motion_helper_match:
    errors.append("缺少 motion 按钮状态解析 helper")
else:
    motion_helper_body = motion_helper_match.group("body")
    for token in [
        "s_bPointerPending",
        "s_tPendingPointer.bButtonDown",
        "left_button_is_down",
    ]:
        if token not in motion_helper_body:
            errors.append(f"motion helper 未覆盖最新按钮状态来源: {token}")

if errors:
    print("FAIL: 阶段 C 仍未满足输入合并/门控基线")
    for item in errors:
        print(f" - {item}")
    sys.exit(1)

print("PASS: 检测到阶段 C 输入合并骨架")
PY
