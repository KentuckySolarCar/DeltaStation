#!/usr/bin/python3

import subprocess
import platform as plat

if plat.uname().system == "Linux":
    subprocess.run(["add-apt-repository", "universe"])
    subprocess.run(["apt", "update"])

    ubuntu_deps = ["libcurl4-openssl-dev", "freeglut3-dev", "libwayland-dev", "libxkbcommon-dev", "xorg-dev"]
    subprocess.run(["apt", "install", "-y"] + ubuntu_deps)
else:
    windows_deps = ["curl"]
    subprocess.run(["vcpkg", "install"] + windows_deps)
