# Undermountain

## Getting Started

### Download

```sh
git clone https://github.com/daviskauffmann/undermountain.git
cd undermountain
```

### Install Dependencies

Install [CMake](https://cmake.org/).
Install [vcpkg](https://vcpkg.io/).

```sh
vcpkg install libtcod
```

### Build

```sh
mkdir build
cd build
cmake ..
```

## Configuration

In `data`, copy `config.txt` and rename to `config.local.txt` to make local changes to the configuration. This file is not source controlled.
