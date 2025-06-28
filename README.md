# Delta Station

University of Kentucky Solar Car data aggregation and display.

## Background

There was an old package written in Python that does exactly what this project can do. However, setting it up with the
default package managers was rather difficult, and in the spirit of preserving a single-language development
environment, we decided to port the project to C++.

## Compiling and Running

This project uses git submodules for pulling dependencies and CMake for compiling the project. To ensure all the latest
packages are pulled, open a terminal in the directory of this README and run:

```shell
git submodule update --init --recursive
```

Once you've updated all your submodules, you need to configure your installation of ImGui. TODO: finish this tutorial.

Once your submodules are successfully cloned, run the following commands to build your CMake project (assumes CMake
version 3.15+):

```shell
mkdir build
cd build
cmake ..
cmake --build .
```

You will then see an executable file called `ds` or `ds.exe` in your build directory.

## TODOs
Note these are in order of importance to the project.
- [x] Dropdowns/widgets for dashboard state instead of plain-text.
- [x] Toggle-able live graphs.
- [ ] More live graphs for battery, GPS data, etc. (past 20 secs)
  - [ ] Power in
  - [ ] Regen (green), throttle (red), and raw pedal percents (past 5 mins)
  - [ ] Array graph
- [ ] Logging configuration for dashboard state.
- [ ] Background image.
- [ ] Minesweeper :D
