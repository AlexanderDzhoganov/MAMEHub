#!/bin/sh

# generate protobufs

# protoc -I=src/emu --cpp_out=src/emu src/emu/nsm.proto
# mv src/emu/nsm.pb.cc src/emu/nsm.pb.c

# build mame

rm mamecps1*

emmake make -j5 LTO=1 OPTIMIZE=3 SYMBOLS=0
mv mamecps1 mamecps1.bc
emcc -O3 -s DISABLE_EXCEPTION_CATCHING=0 -s USE_PTHREADS=0 -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_ZLIB=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 mamecps1.bc -o mamecps1.js

cp mamecps1.js ../emushare/public/
cp mamecps1.wasm ../emushare/public/
