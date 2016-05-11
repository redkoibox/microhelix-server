include_directories(include ../include ../external)

set(SOURCES src/network)
set(HEADERS ../include/microhelix_commons include/network)

source_group("Headers" FILES ${HEADERS})

add_library(network STATIC ${SOURCES} ${HEADERS})
