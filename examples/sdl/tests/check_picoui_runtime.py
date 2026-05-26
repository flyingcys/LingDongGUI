import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / "build-picoui-runtime"
RTK = shutil.which("rtk") or "rtk"

subprocess.run([
    RTK, "cmake", "-S", str(ROOT), "-B", str(BUILD), "-DUSE_DEMO=0"
], check=True)
subprocess.run([
    RTK, "cmake", "--build", str(BUILD), "--target", "picoui_settings_panel_demo"
], check=True)
