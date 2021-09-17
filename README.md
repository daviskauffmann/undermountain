# Undermountain

## Getting Started

### Download

```sh
git clone https://github.com/daviskauffmann/undermountain.git
cd undermountain
```

### Install libtcod

First, install [Vcpkg](https://github.com/microsoft/vcpkg). On Windows, I would recommend setting the `VCPKG_DEFAULT_TRIPLET` environment variable for your system to `x64-windows`, since vcpkg defaults to x64. Come on, its *current year*.

```sh
vcpkg install libtcod
```

I am on Windows using MSYS2, so vcpkg is my only choice for the libtcod dependency. Theoretically, you could build and install libtcod from source whichever way you want and it will most likely work, but this is untested.

### Build

```sh
mkdir build
cd build
cmake ..
make
```

## Configuration

In `data`, copy `config.txt` and rename to `config.local.txt` to make local changes to the configuration. This file is not source controlled.
