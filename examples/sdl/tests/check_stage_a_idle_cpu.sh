#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
ROOT_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
DEMO_PATH="${ROOT_DIR}/build/demo"

SAMPLE_COUNT=${SAMPLE_COUNT:-5}
SAMPLE_INTERVAL=${SAMPLE_INTERVAL:-1.0}
WARMUP_SECONDS=${WARMUP_SECONDS:-1.0}
POST_LOG_SECONDS=${POST_LOG_SECONDS:-1.5}
TOTAL_CPU_HIGH_WATERMARK=${TOTAL_CPU_HIGH_WATERMARK:-140.0}
TOTAL_CPU_HIT_THRESHOLD=${TOTAL_CPU_HIT_THRESHOLD:-120.0}
TOTAL_CPU_MIN_HITS=${TOTAL_CPU_MIN_HITS:-3}
MAIN_THREAD_HIGH_WATERMARK=${MAIN_THREAD_HIGH_WATERMARK:-70.0}
ARM_THREAD_HIGH_WATERMARK=${ARM_THREAD_HIGH_WATERMARK:-25.0}

if [[ ! -x "${DEMO_PATH}" ]]; then
    echo "FAIL: 缺少可执行文件 ${DEMO_PATH}" >&2
    exit 2
fi

work_dir=$(mktemp -d)
log_file="${work_dir}/demo.log"
report_file="${work_dir}/report.txt"
original_dir=$(pwd)

cleanup() {
    cd "${original_dir}" 2>/dev/null || true
    if [[ -n "${demo_pid:-}" ]] && kill -0 "${demo_pid}" 2>/dev/null; then
        kill "${demo_pid}" 2>/dev/null || true
        wait "${demo_pid}" 2>/dev/null || true
    fi
    rm -rf "${work_dir}"
}
trap cleanup EXIT

cd "${ROOT_DIR}"
env SDL_VIDEODRIVER=dummy "${DEMO_PATH}" >"${log_file}" 2>&1 &
demo_pid=$!
cd "${original_dir}"

set +e
python3 - "${demo_pid}" "${log_file}" "${report_file}" \
    "${SAMPLE_COUNT}" "${SAMPLE_INTERVAL}" "${WARMUP_SECONDS}" "${POST_LOG_SECONDS}" \
    "${TOTAL_CPU_HIGH_WATERMARK}" "${TOTAL_CPU_HIT_THRESHOLD}" "${TOTAL_CPU_MIN_HITS}" \
    "${MAIN_THREAD_HIGH_WATERMARK}" "${ARM_THREAD_HIGH_WATERMARK}" <<'PY'
import os
import signal
import subprocess
import sys
import time
from collections import defaultdict

pid = int(sys.argv[1])
log_path = sys.argv[2]
report_path = sys.argv[3]
sample_count = int(sys.argv[4])
sample_interval = float(sys.argv[5])
warmup_seconds = float(sys.argv[6])
post_log_seconds = float(sys.argv[7])
total_cpu_high_watermark = float(sys.argv[8])
total_cpu_hit_threshold = float(sys.argv[9])
total_cpu_min_hits = int(sys.argv[10])
main_thread_high_watermark = float(sys.argv[11])
arm_thread_high_watermark = float(sys.argv[12])

clk_tck = os.sysconf(os.sysconf_names["SC_CLK_TCK"])
cpu_count = os.cpu_count() or 1
page_markers = [
    "page uiWidgetPage1 init",
    "page uiWidgetPage2 init",
    "page uiWidgetPage3 init",
]


def process_exists(target_pid: int) -> bool:
    return os.path.exists(f"/proc/{target_pid}")


def read_total_jiffies() -> int:
    with open("/proc/stat", "r", encoding="utf-8") as handle:
        values = handle.readline().split()[1:]
    return sum(int(value) for value in values)


def read_proc_jiffies(target_pid: int) -> int:
    with open(f"/proc/{target_pid}/stat", "r", encoding="utf-8") as handle:
        parts = handle.read().split()
    return int(parts[13]) + int(parts[14])


def read_threads(target_pid: int):
    tasks = {}
    for tid_text in os.listdir(f"/proc/{target_pid}/task"):
        tid = int(tid_text)
        try:
            with open(f"/proc/{target_pid}/task/{tid}/comm", "r", encoding="utf-8") as handle:
                name = handle.read().strip()
            with open(f"/proc/{target_pid}/task/{tid}/stat", "r", encoding="utf-8") as handle:
                parts = handle.read().split()
            tasks[tid] = {
                "name": name,
                "jiffies": int(parts[13]) + int(parts[14]),
            }
        except FileNotFoundError:
            continue
    return tasks


def scan_pages(path: str):
    hits = []
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as handle:
            for line in handle:
                for marker in page_markers:
                    if marker in line and marker not in hits:
                        hits.append(marker)
    except FileNotFoundError:
        pass
    return hits


deadline = time.time() + 3.0
while time.time() < deadline and not process_exists(pid):
    time.sleep(0.05)

if not process_exists(pid):
    print("FAIL: demo 进程未成功拉起")
    sys.exit(2)

time.sleep(warmup_seconds)

total_samples = []
thread_samples = []
thread_history = defaultdict(list)

prev_total = read_total_jiffies()
prev_proc = read_proc_jiffies(pid)
prev_threads = read_threads(pid)

for index in range(sample_count):
    time.sleep(sample_interval)
    if not process_exists(pid):
        break

    now_total = read_total_jiffies()
    now_proc = read_proc_jiffies(pid)
    now_threads = read_threads(pid)

    delta_total = now_total - prev_total
    delta_proc = now_proc - prev_proc
    if delta_total <= 0:
        total_cpu = 0.0
    else:
        total_cpu = 100.0 * cpu_count * delta_proc / delta_total
    total_samples.append(total_cpu)

    current_thread_rows = []
    for tid, info in now_threads.items():
        old = prev_threads.get(tid)
        if not old:
            continue
        delta_thread = info["jiffies"] - old["jiffies"]
        if delta_total <= 0:
            thread_cpu = 0.0
        else:
            thread_cpu = 100.0 * cpu_count * delta_thread / delta_total
        row = {
            "tid": tid,
            "name": info["name"],
            "cpu": thread_cpu,
        }
        current_thread_rows.append(row)
        thread_history[info["name"]].append(thread_cpu)

    current_thread_rows.sort(key=lambda item: item["cpu"], reverse=True)
    thread_samples.append(current_thread_rows[:5])

    prev_total = now_total
    prev_proc = now_proc
    prev_threads = now_threads

ps_snapshot = ""
try:
    ps_snapshot = subprocess.check_output(
        ["ps", "-L", "-o", "pid,tid,pcpu,comm", "-p", str(pid)],
        text=True,
        stderr=subprocess.STDOUT,
    ).strip()
except subprocess.CalledProcessError as exc:
    ps_snapshot = exc.output.strip()

time.sleep(post_log_seconds)

if process_exists(pid):
    os.kill(pid, signal.SIGTERM)
    time.sleep(0.3)
    if process_exists(pid):
        os.kill(pid, signal.SIGKILL)

pages_hit = scan_pages(log_path)
progression_ok = pages_hit == page_markers

total_hits = sum(1 for value in total_samples if value >= total_cpu_hit_threshold)
total_cpu_fail = False
if total_samples:
    total_cpu_fail = max(total_samples) >= total_cpu_high_watermark or total_hits >= total_cpu_min_hits

main_thread_peak = max(thread_history.get("demo", [0.0]))
arm_thread_peak = max(thread_history.get("arm-2d thread", [0.0]))
thread_sampling_present = bool(thread_samples)
thread_cpu_fail = main_thread_peak >= main_thread_high_watermark and arm_thread_peak >= arm_thread_high_watermark

lines = []
lines.append("=== Stage A Idle CPU Check ===")
lines.append(f"demo_pid: {pid}")
lines.append(
    "config: "
    f"samples={sample_count}, interval={sample_interval}s, warmup={warmup_seconds}s, post_log={post_log_seconds}s"
)
lines.append("")
lines.append("总 CPU 多次采样(%):")
if total_samples:
    for idx, value in enumerate(total_samples, start=1):
        lines.append(f"  sample{idx}: {value:.1f}")
    lines.append(
        f"  summary: max={max(total_samples):.1f}, hits>={total_cpu_hit_threshold:.1f} => {total_hits}/{len(total_samples)}"
    )
else:
    lines.append("  无采样数据")
lines.append("")
lines.append("线程 CPU 多次采样(%):")
if thread_samples:
    for idx, rows in enumerate(thread_samples, start=1):
        if not rows:
            lines.append(f"  sample{idx}: 无线程增量")
            continue
        summary = ", ".join(f"{row['name']}[{row['tid']}]={row['cpu']:.1f}" for row in rows)
        lines.append(f"  sample{idx}: {summary}")
    lines.append(f"  peaks: demo={main_thread_peak:.1f}, arm-2d thread={arm_thread_peak:.1f}")
else:
    lines.append("  无线程采样数据")
lines.append("")
lines.append("页面推进日志命中:")
for marker in page_markers:
    status = "HIT" if marker in pages_hit else "MISS"
    lines.append(f"  {status}: {marker}")
lines.append(f"  progression_ok={str(progression_ok).lower()}")
lines.append("")
lines.append("ps -L 快照:")
lines.append(ps_snapshot or "  无 ps 输出")
lines.append("")
lines.append("判定:")
lines.append(
    f"  total_cpu_fail={str(total_cpu_fail).lower()} "
    f"(max>={total_cpu_high_watermark:.1f} 或 hits>={total_cpu_hit_threshold:.1f} 达到 {total_cpu_min_hits})"
)
lines.append(
    f"  thread_cpu_fail={str(thread_cpu_fail).lower()} "
    f"(demo_peak>={main_thread_high_watermark:.1f} 且 arm_peak>={arm_thread_high_watermark:.1f})"
)

status = "PASS"
reasons = []
if not progression_ok:
    status = "FAIL"
    reasons.append("页面未按顺序推进到 uiWidgetPage3")
if total_cpu_fail:
    status = "FAIL"
    reasons.append("idle 总 CPU 多次采样明显偏高")
if thread_sampling_present and thread_cpu_fail:
    status = "FAIL"
    reasons.append("主线程与 arm-2d thread 都持续吃 CPU")
elif not thread_sampling_present:
    status = "FAIL"
    reasons.append("未拿到线程 CPU 采样证据")

if status == "FAIL" and progression_ok and (total_cpu_fail or thread_cpu_fail):
    reasons.append("页面推进仍正常，说明问题集中在 idle CPU/线程占用")

lines.append("")
lines.append(f"FINAL: {status}")
for reason in reasons:
    lines.append(f"  - {reason}")

text = "\n".join(lines)
with open(report_path, "w", encoding="utf-8") as handle:
    handle.write(text)

print(text)
sys.exit(0 if status == "PASS" else 1)
PY

py_status=$?
set -e
exit "${py_status}"
