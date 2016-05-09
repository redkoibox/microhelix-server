include_directories(include ../include)

set(SOURCES src/scripting)
set(HEADERS ../include/microhelix_commons include/scripting)

source_group("Headers" FILES ${HEADERS})

add_library(scripting STATIC ${SOURCES} ${HEADERS})
