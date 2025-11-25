# Delta Station

University of Kentucky Solar Car data aggregation and display. Go [here](./HOWTO.md) for brief documentation on how to use this.

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

Next, you will need to copy the configuration file from the root directory of
the project to your build directory. This allows DeltaStation to configure
message buffering correctly:

```shell
cp ../sample_config.toml ./config.toml
```

You can now run the project by calling `./ds --debug` or `./ds.exe --debug` from your terminal.

## TODOs
Note these are in order of importance to the project.
- [x] Dropdowns/widgets for dashboard state instead of plain-text.
- [x] Toggle-able live graphs.
- [x] Logging configuration for dashboard state.
- [x] Generated live graphs
- [ ] Logging:
  - [ ] Specify log file
  - [ ] Logging to database? Local MySQL server?
  - [ ] Automatic data compression
- [ ] Baud/port configuration inside Delta Station.
- [ ] Background image.
- [ ] Live GPS Tracking.
- [ ] Minesweeper :D
