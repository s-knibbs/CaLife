# CaLife
<img alt="CaLife Screenshot" src="/CaLife-screen.gif?raw=true" title="Diameoba Automaton" width="400" />

[SDL](https://www.libsdl.org/) based app for running cellular automata.

The app can be launched from the command-line but has also been adapted to run within a browser using [Emscripten](https://kripken.github.io/emscripten-site/index.html).

See a demo of it in [action here](https://s-knibbs.github.io/blog/2016/09/08/experimenting-with-cellular-automata/#demo)

## Building Standalone

To build within a linux environment, first install SDL2. For a debian based distribution:

```sh
~$ sudo apt-get install libsdl2-dev
```

Then run:

```sh
~$ ./build.sh gcc
```

Which outputs the executable to `./build`.

## Building with Emscripten

First, follow the [installation instructions](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html) for your platform to install Emscripten.

Then run:

```
~$ build.sh emscripten
```

Or in Windows:

```
> build.bat
```

The app can then be run using:

```
~$ emrun build/index.html
```

## Usage

Command line usage:

```
Usage: build/calife [-s SURVIVES][-b BORN][-f FILL_PERCENT][-w WIDTH][-H HEIGHT]

  -s  Survive rules, default: '23'
  -b  Born rules, default: '3'
  -f  Initial fill percentage, default: 50
  -w  Output width, default: 800
  -H  Output height, default: 600

  Other options:
  -a  Alive colour in rgb hex, default: ff1a1a
  -d  Dying colour in rgb hex, default: a4e443
  -h  Print this help message and exit
```
