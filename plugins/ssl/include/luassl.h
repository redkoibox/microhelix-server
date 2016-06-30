#ifndef __LUA_SSL_HEADER_INCLUDED__
#define __LUA_SSL_HEADER_INCLUDED__

#include <lua.hpp>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <cstring>
#include <memory>

#include "microhelix_commons.h"

int luassl_newpubkey(lua_State *L);
int luassl_deletepubkey(lua_State *L);
int luassl_verifypubkey(lua_State *L);

#endif
