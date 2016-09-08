#!/bin/bash
if [ "$1" = "emscripten" ]
then
emcc -O2 -s USE_SDL=2 -s \
  EXPORTED_FUNCTIONS="['_exitLoop', '_main', '_setAliveColour', '_setDyingColour']" \
  --pre-js pre.js main.c -o build/index.js
cp ui.html build/index.html
elif [ "$1" = "gcc" ]
then
gcc -O3 -Wall -Wconversion -DNDEBUG main.c -lSDL2 -o build/calife
fi