#!/bin/sh

# generate protobufs
# protoc -I=src/emu --cpp_out=src/emu src/emu/nsm.proto
# mv src/emu/nsm.pb.cc src/emu/nsm.pb.c

# build mame
rm mamecps1*
emmake make -j5 TARGET=mame TARGETOS=emscripten NOWERROR=true NO_USE_QTDEBUG=true NO_USE_MIDI=true NO_X11=true CC=emcc AR=emar LD=emcc SUBTARGET=cps1
mv mamecps1 mamecps1.bc
# emcc -O3 -s USE_PTHREADS=1 -s PTHREAD_POOL_SIZE=4 -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_ZLIB=1 -s ASSERTIONS=1 -s WASM=0 -s TOTAL_MEMORY=134217728 csmamecps1.bc -o csmamecps1.js
emcc -O3 -s DISABLE_EXCEPTION_CATCHING=0 -s USE_PTHREADS=0 -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_ZLIB=1 -s ASSERTIONS=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 mamecps1.bc -o mamecps1.js
