#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
SRC_FILE="${ROOT_DIR}/user/Virtual_TFT_Port.c"
DEMO_PATH="${ROOT_DIR}/build/demo"

if [[ ! -f "${SRC_FILE}" ]]; then
    echo "FAIL: 缺少源码 ${SRC_FILE}" >&2
    exit 2
fi

python3 - "${SRC_FILE}" "${DEMO_PATH}" <<'PY'
import os
import pathlib
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import time

source_path = pathlib.Path(sys.argv[1])
demo_path = pathlib.Path(sys.argv[2])
source = source_path.read_text(encoding="utf-8")
errors = []
notes = []


def extract_function_body(name: str) -> str | None:
    pattern = re.compile(rf"static\s+int\s+{name}\(void\)\s*\{{(?P<body>.*?)\n\}}", re.S)
    match = pattern.search(source)
    if not match:
        return None
    return match.group("body")


helper_body = extract_function_body("VT_sdl_get_event_wait_timeout")
if helper_body is None:
    errors.append("缺少阶段 E 动态等待 helper: VT_sdl_get_event_wait_timeout")
else:
    fast_gate_matches = re.findall(r"bNeedFastWakeup\s*=\s*(.*?);", helper_body, re.S)
    fast_gate_expr = None
    for candidate in fast_gate_matches:
        if any(token in candidate for token in [
            "sdl_refresh_pending",
            "sdl_redraw_pending",
            "s_bPointerPending",
        ]):
            fast_gate_expr = candidate
            break
    if fast_gate_expr is None:
        errors.append("wait helper 未显式计算 bNeedFastWakeup")
    else:
        for token in [
            "sdl_refresh_pending",
            "sdl_redraw_pending",
            "s_bPointerPending",
        ]:
            if token not in fast_gate_expr:
                errors.append(f"快速唤醒门控缺少 {token}")

    return_expr_matches = re.findall(r"return\s+(.*?);", helper_body, re.S)
    final_return_expr = None
    for candidate in return_expr_matches:
        if "bNeedFastWakeup" in candidate:
            final_return_expr = candidate
            break
    if final_return_expr is None:
        errors.append("wait helper 缺少返回表达式")
    else:
        if "VT_REFRESH_WAIT_MS" not in final_return_expr or "VT_IDLE_EVENT_WAIT_MS" not in final_return_expr:
            errors.append("wait helper 未同时覆盖动态/静态等待分支")
        if "bNeedFastWakeup" not in final_return_expr:
            errors.append("wait helper 返回值未受 bNeedFastWakeup 控制")

refresh_match = re.search(
    r"bool\s+VT_sdl_refresh_task\(void\)(?P<body>.*?return\s+!sdl_joined;.*?\n\})",
    source,
    re.S,
)
if not refresh_match:
    errors.append("未找到 VT_sdl_refresh_task")
else:
    refresh_body = refresh_match.group("body")
    if "nEventWaitTimeout = VT_sdl_get_event_wait_timeout();" not in refresh_body:
        errors.append("refresh task 未通过 helper 计算 wait timeout")
    if "SDL_WaitEventTimeout(&event, nEventWaitTimeout)" not in refresh_body:
        errors.append("SDL_WaitEventTimeout 未使用动态 timeout 变量")
    if re.search(r"SDL_WaitEventTimeout\s*\(\s*&event\s*,\s*(?:1000\s*/\s*60|VT_REFRESH_WAIT_MS)\s*\)", refresh_body):
        errors.append("refresh task 仍直接把固定 16ms 常量传给 SDL_WaitEventTimeout")

refresh_wait_match = re.search(r"#define\s+VT_REFRESH_WAIT_MS\s+\((\d+)\s*/\s*(\d+)\)", source)
if not refresh_wait_match:
    errors.append("VT_REFRESH_WAIT_MS 未定义为 1000/Hz 形式")
else:
    numerator = int(refresh_wait_match.group(1))
    denominator = int(refresh_wait_match.group(2))
    if denominator <= 0:
        errors.append("VT_REFRESH_WAIT_MS 分母非法")
    else:
        refresh_wait_ms = numerator // denominator
        if refresh_wait_ms > 20:
            errors.append(f"动态场景等待过慢: {refresh_wait_ms}ms")

idle_wait_match = re.search(r"#define\s+VT_IDLE_EVENT_WAIT_MS\s+(\d+)U", source)
if not idle_wait_match:
    errors.append("VT_IDLE_EVENT_WAIT_MS 未定义为显式毫秒常量")
else:
    idle_wait_ms = int(idle_wait_match.group(1))
    if idle_wait_ms < 100 or idle_wait_ms > 250:
        errors.append(f"静态等待周期不在 100~250ms 预期区间: {idle_wait_ms}ms")
    if refresh_wait_match and idle_wait_ms <= (numerator // denominator):
        errors.append("静态等待周期未明显放宽，仍接近固定 16ms")


def run_runtime_trace() -> tuple[bool, str]:
    if not demo_path.is_file() or not os.access(demo_path, os.X_OK):
        return True, "SKIP: 缺少 build/demo，仅执行源码级校验"

    strace_path = shutil.which("strace")
    if not strace_path:
        return True, "SKIP: 未安装 strace，仅执行源码级校验"

    with tempfile.TemporaryDirectory(prefix="stage-e-trace-") as work_dir:
        trace_file = pathlib.Path(work_dir) / "trace.log"
        log_file = pathlib.Path(work_dir) / "demo.log"
        command = [
            strace_path,
            "-tt",
            "-T",
            "-f",
            "-e",
            "trace=ppoll,poll,select,pselect6,epoll_wait,epoll_pwait,nanosleep,clock_nanosleep",
            "-o",
            str(trace_file),
            str(demo_path),
        ]
        env = os.environ.copy()
        env["SDL_VIDEODRIVER"] = env.get("SDL_VIDEODRIVER", "dummy")

        proc = subprocess.Popen(
            command,
            cwd=demo_path.parent.parent,
            stdout=log_file.open("w", encoding="utf-8"),
            stderr=subprocess.STDOUT,
            env=env,
        )
        try:
            time.sleep(1.5)
        finally:
            if proc.poll() is None:
                proc.send_signal(signal.SIGTERM)
                try:
                    proc.wait(timeout=1.0)
                except subprocess.TimeoutExpired:
                    proc.kill()
                    proc.wait(timeout=1.0)

        if not trace_file.exists():
            return True, "SKIP: 未生成 strace 输出，仅执行源码级校验"

        trace_text = trace_file.read_text(encoding="utf-8", errors="replace")
        wait_values = []
        poll_patterns = [
            re.compile(r"ppoll\([^)]*,\s*\{tv_sec=(\d+), tv_nsec=(\d+)\}", re.S),
            re.compile(r"pselect6\([^)]*,\s*\{tv_sec=(\d+), tv_nsec=(\d+)\}", re.S),
            re.compile(r"poll\([^)]*,\s*(\d+)\)\s*=", re.S),
            re.compile(r"select\([^)]*,\s*\{tv_sec=(\d+), tv_usec=(\d+)\}", re.S),
        ]

        for line in trace_text.splitlines():
            for pattern in poll_patterns[:2]:
                match = pattern.search(line)
                if match:
                    sec = int(match.group(1))
                    nsec = int(match.group(2))
                    wait_values.append(sec * 1000 + nsec / 1_000_000)
                    break
            else:
                match = poll_patterns[2].search(line)
                if match:
                    wait_values.append(float(match.group(1)))
                    continue
                match = poll_patterns[3].search(line)
                if match:
                    sec = int(match.group(1))
                    usec = int(match.group(2))
                    wait_values.append(sec * 1000 + usec / 1000)

        if not wait_values:
            return True, "SKIP: strace 未捕获到可解析的等待参数，仅执行源码级校验"

        slow_waits = [value for value in wait_values if value >= 80.0]
        fast_waits = [value for value in wait_values if value <= 25.0]
        summary = (
            f"runtime waits: samples={len(wait_values)}, "
            f"fast<=25ms={len(fast_waits)}, slow>=80ms={len(slow_waits)}, "
            f"max={max(wait_values):.1f}ms"
        )

        if not slow_waits:
            return False, f"运行时 trace 未观察到放宽后的静态等待; {summary}"

        return True, f"运行时 trace 观察到静态长等待; {summary}"


if errors:
    print("FAIL: 阶段 E 动态等待策略未满足")
    for item in errors:
        print(f" - {item}")
    sys.exit(1)

ok, runtime_note = run_runtime_trace()
notes.append(runtime_note)
if not ok:
    print("FAIL: 阶段 E 运行时等待证据未满足")
    print(f" - {runtime_note}")
    sys.exit(1)

print("PASS: 检测到阶段 E 动态/静态 wait 门控")
for item in notes:
    print(f" - {item}")
PY
