"""Validate the standalone HUNTER 2.0 browser demo and its JavaScript syntax."""
from pathlib import Path
import re
import subprocess
import tempfile

html_path = Path("Demo/index.html")
if not html_path.is_file():
    raise SystemExit("Missing Demo/index.html")

html = html_path.read_text(encoding="utf-8")
scripts = re.findall(r"<script>([\s\S]*?)</script>", html)
if len(scripts) != 1:
    raise SystemExit(f"Expected one inline game script, found {len(scripts)}")

required = [
    "THE BREACH", "RUINED DISTRICT", "THE GATEKEEPER",
    "BREACH GUARDIAN", "function slash()", "function energy()",
    "function dash()", "function heal()", "function togglePause()",
    "localStorage"
]
missing = [item for item in required if item not in html]
if missing:
    raise SystemExit("Browser demo is missing required features: " + ", ".join(missing))

with tempfile.NamedTemporaryFile("w", suffix=".js", encoding="utf-8", delete=False) as js_file:
    js_file.write(scripts[0])
    js_path = Path(js_file.name)

try:
    result = subprocess.run(["node", "--check", str(js_path)], text=True, capture_output=True)
    if result.returncode:
        raise SystemExit(result.stderr or "Node.js syntax check failed")
finally:
    js_path.unlink(missing_ok=True)

print("Browser demo validation passed: campaign features and JavaScript syntax are valid.")
