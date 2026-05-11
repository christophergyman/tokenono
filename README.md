# tokenono

A minimal 2D game in C using [raylib](https://www.raylib.com/).

Currently renders a square on screen as the starting point for iteration.

Includes a small fail-fast arena memory foundation with permanent and frame
arenas. A startup arena smoke test runs by default.

## Prerequisites

- macOS (Apple Silicon)
- [Homebrew](https://brew.sh)
- raylib: `brew install raylib`

## Build & Run

```bash
./build
```

Skip startup debug checks:

```bash
./build --no-debug
```

The Makefile is the build source of truth:

```bash
make -f scripts/Makefile run
make -f scripts/Makefile run ARGS="--no-debug"
```

## Development Auto-Restart

Run the dev loop:

```bash
./dev
```

Or through Make:

```bash
make -f scripts/Makefile dev
```

The dev loop watches C sources, headers, `scripts/Makefile`, and `build`. When
one changes, it rebuilds and restarts the game. If a rebuild fails, the current
running game stays open.

Hit `ESC` or close the window to quit the game. Press `Ctrl-C` in the terminal
to stop the dev loop.
