import json
import shutil
import subprocess
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
BUILD_DIR = ROOT / "build" / "picoui-no-ldgui-contract"
QUERY_DIR = BUILD_DIR / ".cmake" / "api" / "v1" / "query"
REPLY_DIR = BUILD_DIR / ".cmake" / "api" / "v1" / "reply"

BANNED_TARGETS = {
    "picoui_backend_ldgui",
    "picoui_backend_ldgui_runtime",
}
BANNED_SOURCE_PREFIXES = (
    "src/gui/ld",
    "picoui/src/backend/ldgui/",
)
TARGET_PREFIXES = (
    "picoui_",
    "test_picoui_",
    "check_picoui_",
)
EXCLUDED_TARGETS = {
    "picoui_backend_ldgui",
    "picoui_backend_ldgui_runtime",
}


def _configure_file_api_build() -> None:
    QUERY_DIR.mkdir(parents=True, exist_ok=True)
    if REPLY_DIR.exists():
        shutil.rmtree(REPLY_DIR, ignore_errors=True)
    (QUERY_DIR / "codemodel-v2").write_text("", encoding="utf-8")
    subprocess.run(
        ["cmake", "-S", str(ROOT), "-B", str(BUILD_DIR)],
        check=True,
        cwd=ROOT,
    )


def _latest_index_file() -> Path | None:
    index_files = sorted(REPLY_DIR.glob("index-*.json"))
    if not index_files:
        return None
    return index_files[-1]


def _wait_for_file(path: Path, attempts: int = 8, delay_s: float = 0.1) -> bool:
    for _ in range(attempts):
        if path.is_file():
            return True
        time.sleep(delay_s)
    return path.is_file()


def _resolve_codemodel_path(index: dict) -> Path | None:
    reply = index.get("reply")
    if not isinstance(reply, dict):
        return None
    codemodel_info = reply.get("codemodel-v2")
    if not isinstance(codemodel_info, dict):
        return None
    codemodel_file = codemodel_info.get("jsonFile")
    if not isinstance(codemodel_file, str):
        return None
    codemodel_path = REPLY_DIR / codemodel_file
    if _wait_for_file(codemodel_path):
        return codemodel_path

    fallback_files = sorted(REPLY_DIR.glob("codemodel-v2-*.json"))
    if fallback_files:
        fallback_path = fallback_files[-1]
        if _wait_for_file(fallback_path):
            return fallback_path
    return None


def _load_codemodel(retry: bool = True) -> tuple[list[dict], dict[str, dict]]:
    index_file = _latest_index_file()
    if index_file is None:
        if retry:
            _configure_file_api_build()
            return _load_codemodel(retry=False)
        raise AssertionError(f"missing CMake file-api index in {REPLY_DIR}")
    index = json.loads(index_file.read_text(encoding="utf-8"))
    codemodel_path = _resolve_codemodel_path(index)
    if codemodel_path is None:
        if retry:
            _configure_file_api_build()
            return _load_codemodel(retry=False)
        raise AssertionError(f"missing codemodel reply in {REPLY_DIR}")
    codemodel = json.loads(codemodel_path.read_text(encoding="utf-8"))
    config = codemodel["configurations"][0]
    targets = config["targets"]
    missing_target_files = [
        target["jsonFile"]
        for target in targets
        if _should_check_target(target["name"])
        and not _wait_for_file(REPLY_DIR / target["jsonFile"])
    ]
    if missing_target_files:
        if retry:
            _configure_file_api_build()
            return _load_codemodel(retry=False)
        raise AssertionError(
            f"missing target replies in {REPLY_DIR}: {missing_target_files[:8]}"
        )
    by_id = {target["id"]: target for target in targets}
    return targets, by_id


def _target_json(target: dict) -> dict:
    return json.loads((REPLY_DIR / target["jsonFile"]).read_text(encoding="utf-8"))


def _should_check_target(name: str) -> bool:
    return name.startswith(TARGET_PREFIXES) and name not in EXCLUDED_TARGETS


def _walk_dependencies(target_id: str, by_id: dict[str, dict], cache: dict[str, set[str]]) -> set[str]:
    if target_id in cache:
        return cache[target_id]

    target = by_id[target_id]
    target_json = _target_json(target)
    deps = {target["name"]}
    for dep in target_json.get("dependencies", []):
        dep_id = dep["id"]
        dep_name = dep_id.split("::", 1)[0]
        deps.add(dep_name)
        if dep_id in by_id:
            deps.update(_walk_dependencies(dep_id, by_id, cache))
    cache[target_id] = deps
    return deps


def _collect_banned_sources(target_json: dict) -> list[str]:
    banned: list[str] = []
    for source in target_json.get("sources", []):
        path = source.get("path")
        if not isinstance(path, str):
            continue
        normalized = path.replace("\\", "/")
        if normalized.startswith(BANNED_SOURCE_PREFIXES):
            banned.append(normalized)
    return banned


def main() -> int:
    _configure_file_api_build()
    targets, by_id = _load_codemodel()
    dep_cache: dict[str, set[str]] = {}
    failures: list[str] = []

    for target in targets:
        name = target["name"]
        if not _should_check_target(name):
            continue

        target_json = _target_json(target)
        dep_closure = _walk_dependencies(target["id"], by_id, dep_cache)
        banned_deps = sorted(dep_closure & BANNED_TARGETS)
        banned_sources = _collect_banned_sources(target_json)

        if banned_deps or banned_sources:
            parts: list[str] = [f"target '{name}' still depends on ldgui runtime by default"]
            if banned_deps:
                parts.append(f"banned_deps={banned_deps}")
            if banned_sources:
                parts.append(f"banned_sources={banned_sources[:8]}")
            failures.append("; ".join(parts))

    assert not failures, "\n".join(failures)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
