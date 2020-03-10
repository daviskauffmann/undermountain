# Undermountain

## Getting Started

### Download

```sh
git clone https://github.com/daviskauffmann/undermountain.git
cd undermountain
git submodule update --init
```

### Build libtcod

```sh
make build_libtcod
```

I am on Windows using MSYS2, so that is the only development platform supported for building libtcod at this time. If the above command doesn't work, you can go into `extern/libtcod` and build it yourself for your platform. The Makefile paths will then most likely need to be updated in order to properly link the library and copy the dll to `bin`.

### Build

```sh
make
```

### Build & Run

```sh
make run
```

### Cleanup

```sh
make clean
```

### Cleanup libtcod

```sh
make clean_libtcod
```

## Configuration

In `data`, copy `config.txt` and rename to `config.local.txt` to make local changes to the configuration. This file is not source controlled.
