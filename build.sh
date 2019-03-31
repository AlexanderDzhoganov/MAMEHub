#!/bin/sh

# generate protobufs

# protoc -I=src/emu --cpp_out=src/emu src/emu/nsm.proto
# mv src/emu/nsm.pb.cc src/emu/nsm.pb.c

# build mame

rm mamecps1*

emmake make -j5 EM_DEBUG=1 OPTIMIZE=3

mv mamecps1 mamecps1.bc

MEMORY=268435456
STACK=16777216
EXPORTED_FN='["_main", "_jsGetNextPacket", "_jsEnqueuePacket"]'

emcc \
  -g -O3 \
  -s USE_PTHREADS=0 -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_ZLIB=1 \
  -s ASSERTIONS=2 -s WASM=1 \
  -s EXPORTED_FUNCTIONS="$EXPORTED_FN" \
  -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
  -s TOTAL_MEMORY="$MEMORY" -s TOTAL_STACK="$STACK" \
  -s DETERMINISTIC=1 -s EMULATE_FUNCTION_POINTER_CASTS=1 -s DISABLE_EXCEPTION_CATCHING=0 \
  mamecps1.bc -o mamecps1.js

cp mamecps1.js ../emushare/public/
cp mamecps1.wasm ../emushare/public/
