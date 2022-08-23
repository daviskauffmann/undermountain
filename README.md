# Undermountain

## Getting Started

### Download

```sh
git clone --recurse-submodules https://github.com/daviskauffmann/undermountain.git
cd undermountain
```

### Build

```sh
mkdir build
cd build
cmake ..
```

## Configuration

In `data`, copy `config.cfg` and rename to `config.local.cfg` to make local changes to the configuration. This file is not source controlled. Some configuration values can be controlled by command line arguments as well. Run with `--help` to view them. Command line arguments will override config file values for the same setting.

## Resources Used

[A Turn-Based Game Loop](http://journal.stuffwithstuff.com/2014/07/15/a-turn-based-game-loop/)
