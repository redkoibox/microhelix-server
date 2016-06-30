include_directories(include ../../include)

set(SOURCES src/luassl)
set(HEADERS ../../include/microhelix_commons include/luassl)

add_definitions(-DBUILDING_DLL)

source_group("Headers" FILES ${HEADERS})

add_library(luassl SHARED ${SOURCES} ${HEADERS} ${RAPIDJSON})
target_link_libraries(luassl ${LUA_LIBRARY})
target_link_libraries(luassl ${OPENSSL_LIBRARIES})

set_property(TARGET luassl PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
if(UNIX)
	install(TARGETS luassl
			LIBRARY DESTINATION microhelix/plugins)
else()
	install(TARGETS luassl
			LIBRARY DESTINATION microhelix/plugins
			RUNTIME DESTINATION microhelix/plugins)
endif(UNIX)
