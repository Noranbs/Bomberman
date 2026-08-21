# Bomberman AP

Advanced Programming project: a small Bomberman style game written in C++20 with SFML.

## Student

- Name: TODO
- Student number: TODO
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

## Game

The game starts with a menu. The menu shows the top five scores and has a Play button. There is also an instructions screen with controls and power-ups.

The player starts in the top left corner. The three enemies start in the other corners. The goal is to defeat all enemies. After all enemies are defeated, an exit appears. The player can go to the exit immediately or first collect more power-ups.

## Documentation

The full project report is in [REPORT.md](REPORT.md).
