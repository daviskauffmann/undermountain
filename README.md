# Undermountain

## Getting Started

I am on Windows using MinGW64, so that is the only development platform supported at this time.

Install third party libraries.

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
