include_directories(include ../include ../network/include ../scripting/include ../external)

set(SOURCES src/main)
set(HEADERS ../include/microhelix_commons)

source_group("Headers" FILES ${HEADERS})

add_executable(microhelix ${SOURCES} ${HEADERS})
target_link_libraries(microhelix network)
target_link_libraries(microhelix scripting)
target_link_libraries(microhelix ${Boost_LIBRARIES})
target_link_libraries(microhelix ${OPENSSL_LIBRARIES})
target_link_libraries(microhelix ${LUA_LIBRARY})
if(CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(microhelix dl)
endif(CMAKE_COMPILER_IS_GNUCC)

set_property(TARGET microhelix PROPERTY INSTALL_RPATH_USE_LINK_PATH TRUE)
install(TARGETS microhelix
        RUNTIME DESTINATION microhelix/bin)
