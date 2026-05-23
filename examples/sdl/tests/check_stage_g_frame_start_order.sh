#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
SRC_FILE="${ROOT_DIR}/../../src/gui/ldGui.c"

if [[ ! -f "${SRC_FILE}" ]]; then
    echo "FAIL: 缺少源码 ${SRC_FILE}" >&2
    exit 2
fi

python3 - "${SRC_FILE}" <<'PY'
import pathlib
import re
import sys

source_path = pathlib.Path(sys.argv[1])
source = source_path.read_text(encoding="utf-8")

signature = re.search(r"void\s+ldGuiFrameStart\(ld_scene_t\s+\*ptScene\)\s*\{", source)
if not signature:
    print("FAIL: 未找到 ldGuiFrameStart")
    sys.exit(1)

start = signature.end() - 1
brace_depth = 0
end = None
for index in range(start, len(source)):
    ch = source[index]
    if ch == "{":
        brace_depth += 1
    elif ch == "}":
        brace_depth -= 1
        if brace_depth == 0:
            end = index
            break

if end is None:
    print("FAIL: 无法解析 ldGuiFrameStart 函数体")
    sys.exit(1)

body = source[start + 1:end]

loop_pos = body.find("ptScene->ldGuiFuncGroup->loop(ptScene);")
page_frame_start_pos = body.find("ptScene->ldGuiFuncGroup->frameStart(ptScene);")
widget_frame_start_pos = body.find("((ldBase_t*)ptItem)->ptGuiFunc->frameStart(ptScene,ptItem);")

errors = []
if loop_pos == -1:
    errors.append("缺少 page loop 调用")
if page_frame_start_pos == -1:
    errors.append("缺少 page frameStart 调用")
if widget_frame_start_pos == -1:
    errors.append("缺少 widget frameStart 调用")

if not errors:
    if loop_pos > widget_frame_start_pos:
        errors.append("page loop 仍晚于 widget frameStart，layout 变更会延后一帧生效")
    if loop_pos > page_frame_start_pos:
        errors.append("page loop 仍晚于 page frameStart")

if errors:
    print("FAIL: 阶段 G frameStart 顺序未满足")
    for item in errors:
        print(f" - {item}")
    sys.exit(1)

print("PASS: 检测到 page loop 先于 frameStart 执行")
PY
