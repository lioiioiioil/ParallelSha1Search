"""Check the platform's base implementations against known seven-byte inputs."""
import argparse
import hashlib
import os
from pathlib import Path
import subprocess

p = argparse.ArgumentParser(description=__doc__)
p.add_argument("--build-dir", type=Path, default=Path("build"))
a = p.parse_args()
targets = ["serial", "std_thread"]
targets += ["create_thread", "create_process"] if os.name == "nt" else ["pthread_search", "fork_search"]
for target in targets:
    exe = (a.build_dir/(target+(".exe" if os.name == "nt" else ""))).resolve()
    if not exe.is_file():
        p.error(f"Executable not found: {exe}")
    for password in ["0000000", "000000a", "0000010"]:
        digest = hashlib.sha1(password.encode("ascii")).hexdigest()
        result = subprocess.run([str(exe)], input=digest+"\n", text=True, capture_output=True, timeout=20, check=True)
        answer = result.stdout.strip().split()[-1]
        if answer != password:
            raise SystemExit(f"FAIL {target}: expected {password!r}, got {result.stdout!r}")
    print(f"PASS {target}: 3 known inputs")
