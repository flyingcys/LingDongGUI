#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
SRC_FILE="${ROOT_DIR}/user/Virtual_TFT_Port.c"

motion_block=$(
    awk '
        /case SDL_MOUSEMOTION:/ {in_block=1}
        in_block {print}
        in_block && /^[[:space:]]*break;[[:space:]]*$/ {exit}
    ' "${SRC_FILE}"
)

if printf '%s\n' "${motion_block}" | rg -q 'last_[xy][[:space:]]*='; then
    echo "FAIL: SDL_MOUSEMOTION 仍直接写 committed 坐标"
    exit 1
fi

for pattern in \
    's_tPendingPointer' \
    's_bPointerPending' \
    's_nLastPointerCommitTick' \
    'VT_sdl_commit_pending_pointer' \
    'VT_sdl_resolve_motion_button_locked'
do
    if ! rg -q "${pattern}" "${SRC_FILE}"; then
        echo "FAIL: 缺少阶段 C 关键状态或提交逻辑: ${pattern}"
        exit 1
    fi
done

if ! awk '
        /case SDL_MOUSEBUTTONDOWN:/ {in_block=1}
        in_block {print}
        in_block && /^[[:space:]]*break;[[:space:]]*$/ {exit}
    ' "${SRC_FILE}" | rg -q 'VT_sdl_stage_pointer_locked'; then
    echo "FAIL: SDL_MOUSEBUTTONDOWN 未走 pending stage 路径"
    exit 1
fi

if ! awk '
        /case SDL_MOUSEBUTTONUP:/ {in_block=1}
        in_block {print}
        in_block && /^[[:space:]]*break;[[:space:]]*$/ {exit}
    ' "${SRC_FILE}" | rg -q 'VT_sdl_stage_pointer_locked'; then
    echo "FAIL: SDL_MOUSEBUTTONUP 未走 pending stage 路径"
    exit 1
fi

if ! awk '
        /bool VT_sdl_refresh_task\(void\)/ {in_func=1}
        in_func {print}
        in_func && /^[[:space:]]*return !sdl_joined;[[:space:]]*$/ {done=1}
        done && /^[[:space:]]*}[[:space:]]*$/ {exit}
    ' "${SRC_FILE}" | rg -q 'VT_sdl_commit_pending_pointer\(bNeedRedraw\)'; then
    echo "FAIL: refresh 周期内未见 pending 输入提交"
    exit 1
fi

if ! awk '
        /bool VT_mouse_get_location\(arm_2d_location_t \*ptLocation\)/ {in_func=1}
        in_func {print}
        in_func && /^[[:space:]]*return bPressed;[[:space:]]*$/ {done=1}
        done && /^[[:space:]]*}[[:space:]]*$/ {exit}
    ' "${SRC_FILE}" | rg -q 'ptLocation->iX = last_x'; then
    echo "FAIL: VT_mouse_get_location 未明确读取 committed 坐标"
    exit 1
fi

echo "PASS: 阶段 C 输入合并与提交结构存在"
