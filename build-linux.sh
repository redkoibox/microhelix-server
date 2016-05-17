#!/bin/bash
mkdir build && cd build
export LUA_DIR=/usr/local
cmake ../ -DLIBBSON_DIR=/usr/local -DLIBMONGOC_DIR=/usr/local
