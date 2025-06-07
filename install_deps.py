#!/usr/bin/python3

import subprocess
import platform as plat

if plat.uname().system == "Linux":
    ubuntu_deps = ["freeglut3-dev", "libwayland-dev", "libxkbcommon-dev", "xorg-dev"]
    subprocess.run(["apt", "install", "-y"] + ubuntu_deps)
else:
    print("Windows not configured for install!")
    #raise NotImplementedError("Windows configure not implemented yet.")
