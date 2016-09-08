@echo off
call emcc -O2 -s USE_SDL=2 -s ^
  EXPORTED_FUNCTIONS="['_exitLoop', '_main', '_setAliveColour', '_setDyingColour']" ^
  --pre-js pre.js main.c -o build\index.js
copy ui.html build\index.html