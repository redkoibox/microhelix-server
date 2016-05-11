include_directories(include ../include ../network/include ../external)

set(SOURCES src/scripting src/script src/http_helpers)
set(HEADERS ../include/microhelix_commons include/scripting include/script include/http_helpers)
file(GLOB RAPIDJSON ../external/rapidjson/*.h)

source_group("Headers" FILES ${HEADERS})
source_group("RapidJSON" FILES ${RAPIDJSON})

add_library(scripting STATIC ${SOURCES} ${HEADERS} ${RAPIDJSON})
