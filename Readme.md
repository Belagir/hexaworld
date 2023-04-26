# Hexaworld

A small raylib-powered c program that will generate an hexagonally-tiled world through successive application of cellular automatons.

![hexaworld icon, a frog vibing on a tile](img/icon/hexaworld.png)

## Compilation

### Prerequisites

Having `raylib` installed on the system as a static library.
Having `gcc` and `make` installed on the system.

### Command

position yourself at the project's root and type :

```bash
$ make
```

... to compile the project. This will create a `build` directory (not very interesting) and a `bin` directory (mucho more interesting).

### Result

You will find the binary executable (`otomaton`) in the bin directory :

```
.
+-- bin
|   +-- otomaton
+-- build
│   +-- ...
+-- img
│   +-- ...
+-- inc
│   +-- ...
+-- src
    +-- ...
+-- Makefile
+-- Readme.md

```

## Usage

Execute the program :
```bash
$ ./bin/otomaton
```

You can navigate the layers with the left and right arrows. You also can re-generate the map with left shift + enter.

The program accepts some options :

- `-s seed` with `seed` as any integer. The program will use this to seed the RNG.
- `-x width` with `width` as a non-zero unsigned integer. This will set the horizontal number of tiles.
- `-y height` with `height` as a non-zero unsigned integer. This will set the vertical number of tiles.
