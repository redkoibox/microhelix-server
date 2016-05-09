include_directories(include ../include)

set(SOURCES src/network)
set(HEADERS ../include/microhelix_commons include/network include/client_http include/client_https include/server_http include/server_https)

source_group("Headers" FILES ${HEADERS})

add_library(network STATIC ${SOURCES} ${HEADERS})
