# Bomberman AP

Advanced Programming project: a small Bomberman style game written in C++20 with SFML.

## Student

- Name: TODO
- Student number: TODO
- GitHub repository: TODO

## Build and Run

The project uses CMake. The game logic is built as a separate library, and the SFML application links against it.

```bash
cmake -S . -B build
cmake --build build
./build/Bomberman_AP
```

To run the logic tests:

```bash
cd build
ctest
```

To generate Doxygen documentation:

```bash
cd build
make doc
```

The generated documentation is written to `build/doc/html/index.html`.

## Controls

- Move: arrow keys or WASD
- Place bomb: Space
- Kick bomb after collecting Punch Glove: K
- Pause/resume: Enter
- Return to menu while paused: Esc

## Documentation

The full project report is in [REPORT.md](REPORT.md).
