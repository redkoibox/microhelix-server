#!/bin/bash
mkdir build
cd build
export LUA_DIR=/usr/local
cmake -G "Unix Makefiles" -DLIBBSON_DIR=/usr/local -DLIBMONGOC_DIR=/usr/local -DCMAKE_INSTALL_PREFIX=/tmp/builds -DMICROHELIX_INSTALL_DIR=/usr/local ../
make
make install
mkdir compiled
tar czvf ./compiled/microhelix.tar.gz /usr/local/microhelix