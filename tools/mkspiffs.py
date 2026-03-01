from SCons.Script import Import
Import("env")
import os
from shutil import copyfile

# Ensure the data directory exists or use a fallback
DATA_DIR = "data"
if not os.path.exists(DATA_DIR):
    # Try creating it if it was deleted/renamed
    os.makedirs(DATA_DIR, exist_ok=True)
    with open(os.path.join(DATA_DIR, "index.html"), "w") as f:
        f.write("<html><body><h1>Pot Connecte</h1></body></html>")

spiffs_files = [
    (os.path.join(DATA_DIR, "index.html"), "$BUILD_DIR/data.index.html"),
    (os.path.join(DATA_DIR, "style.css"), "$BUILD_DIR/data.style.css"),
    (os.path.join(DATA_DIR, "main.js"), "$BUILD_DIR/data.main.js"),
]

for src, dst in spiffs_files:
    dst_expanded = env.subst(dst)
    os.makedirs(os.path.dirname(dst_expanded), exist_ok=True)
    if os.path.exists(src):
        copyfile(src, dst_expanded)
        print(f"CI: Copied {src} to {dst_expanded}")
    else:
        print(f"CI Warning: {src} not found!")
