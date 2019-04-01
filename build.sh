#!/bin/sh

# generate protobufs

# protoc -I=src/emu --cpp_out=src/emu src/emu/nsm.proto
# mv src/emu/nsm.pb.cc src/emu/nsm.pb.c

# build mame

rm mamecps1*

emmake make -j5 OPTIMIZE=3 MAME_DEBUG=1

mv mamecps1 mamecps1.bc

EXPORTED_FN='["_main", "_malloc", "_jsGetNextPacket", "_jsEnqueuePacket"]'

emcc \
  -g -O3 \
  -s USE_PTHREADS=0 -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_ZLIB=1 \
  -s ASSERTIONS=2 -s WASM=1 -s DEMANGLE_SUPPORT=1 \
  -s EXPORTED_FUNCTIONS="$EXPORTED_FN" \
  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
  -s TOTAL_MEMORY=256MB -s TOTAL_STACK=8MB \
  -s DISABLE_EXCEPTION_CATCHING=0 \
  -s DETERMINISTIC=1 \
  mamecps1.bc -o mamecps1.js

cp mamecps1.js ../emushare/public/
cp mamecps1.wasm ../emushare/public/
