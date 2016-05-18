include_directories(include ../../include ../../external)

set(SOURCES src/luamongo src/json_utils)
set(HEADERS ../../include/microhelix_commons include/luamongo include/json_utils)
file(GLOB RAPIDJSON ../../external/rapidjson/*.h)

add_definitions(-DBUILDING_DLL)

find_package(LibBSON REQUIRED)
find_package(LibMongoC REQUIRED)

include_directories(${LIBBSON_INCLUDE_DIRS})
include_directories(${LIBMONGOC_INCLUDE_DIRS})

link_directories(${LIBBSON_LIBRARY_DIRS})
link_directories(${LIBMONGOC_LIBRARY_DIRS})

source_group("Headers" FILES ${HEADERS})
source_group("RapidJSON" FILES ${RAPIDJSON})

add_library(mongodb SHARED ${SOURCES} ${HEADERS} ${RAPIDJSON})
target_link_libraries(mongodb ${LIBMONGOC_LIBRARIES})
target_link_libraries(mongodb ${LIBBSON_LIBRARIES})
target_link_libraries(mongodb ${LUA_LIBRARY})

set_property(TARGET mongodb PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS mongodb
        LIBRARY DESTINATION microhelix/plugins)
