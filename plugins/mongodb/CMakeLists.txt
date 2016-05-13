include_directories(include ../../include)

set(SOURCES src/luamongo)
set(HEADERS ../../include/microhelix_commons include/luamongo)

add_definitions(-DBUILDING_DLL)

find_package(LibBSON REQUIRED)
find_package(LibMongoC REQUIRED)

include_directories(${LIBBSON_INCLUDE_DIRS})
include_directories(${LIBMONGOC_INCLUDE_DIRS})

link_directories(${LIBBSON_LIBRARY_DIRS})
link_directories(${LIBMONGOC_LIBRARY_DIRS})

add_library(mongodb SHARED ${SOURCES} ${HEADERS})
target_link_libraries(mongodb ${LIBMONGOC_LIBRARIES})
target_link_libraries(mongodb ${LIBBSON_LIBRARIES})
target_link_libraries(mongodb ${LUA_LIBRARY})
