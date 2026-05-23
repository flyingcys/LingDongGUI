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

source_path = pathlib.Path(sys.argv[1])
source = source_path.read_text(encoding="utf-8")
errors = []

signature = re.search(
    r"static\s+bool\s+VT_sdl_present_texture\(const\s+arm_2d_region_t\s+\*ptDirtyRegion\)\s*\{",
    source,
)
if not signature:
    print("FAIL: 未找到 VT_sdl_present_texture")
    sys.exit(1)

start = signature.end() - 1
brace_depth = 0
end = None
for index in range(start, len(source)):
    ch = source[index]
    if ch == '{':
        brace_depth += 1
    elif ch == '}':
        brace_depth -= 1
        if brace_depth == 0:
            end = index
            break

if end is None:
    print("FAIL: 无法完整解析 VT_sdl_present_texture 函数体")
    sys.exit(1)

body = source[start + 1:end]

if "SDL_RenderClear(renderer)" not in body:
    errors.append("present 路径缺少 SDL_RenderClear(renderer)")

if not re.search(r"SDL_RenderCopy\s*\(\s*renderer\s*,\s*texture\s*,\s*NULL\s*,\s*NULL\s*\)", body):
    errors.append("present 路径未使用 SDL_RenderCopy(renderer, texture, NULL, NULL) 全量绘制")

if re.search(r"SDL_RenderCopy\s*\(\s*renderer\s*,\s*texture\s*,\s*&tRect\s*,\s*&tRect\s*\)", body):
    errors.append("present 路径仍在使用 SDL_RenderCopy(renderer, texture, &tRect, &tRect)")

if not re.search(r"SDL_UpdateTexture\s*\(\s*texture\s*,\s*&tRect\s*,", body):
    errors.append("dirty rect 纹理更新路径丢失 SDL_UpdateTexture(texture, &tRect, ...)")

if errors:
    print("FAIL: 阶段 F full present 约束未满足")
    for item in errors:
        print(f" - {item}")
    sys.exit(1)

print("PASS: present 路径满足 full render 约束")
PY
