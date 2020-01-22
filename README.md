# Undermountain

## Getting Started

Download and install libtcod. I am on Windows using MinGW64, so that is the only development platform supported for building libtcod at this time. If the following commands don't work, you can go into `/deps/libtcod` and build it yourself for your platform. The Makefile paths will then most likely need to be updated in order to properly link the library and copy the dll to `/bin`.

```sh
git submodule update --init
make setup
```

Build the project.

```sh
make
```

Build and run.

```sh
make run
```

Cleanup all build files and binaries.

```sh
make clean
```

## Configuration

Copy `config.txt` and rename to `config.local.txt` to make local changes to the configuration. This file is not source controlled.
