cmake_minimum_required(VERSION 3.2.3)
project(microhelix)

if (MSVC)
	add_definitions(-D_WIN32_WINNT=0x0501)
	add_definitions(-DBOOST_ALL_NO_LIB)
endif(MSVC)

if(CMAKE_COMPILER_IS_GNUCC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O3 -Wall")
endif(CMAKE_COMPILER_IS_GNUCC)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_BUILD_TYPE Release)

if (MSVC)
	set(BOOST_LIBRARYDIR "$ENV{BOOST_ROOT}\\stage\\lib")
endif (MSVC)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.60.0 COMPONENTS regex system thread coroutine context filesystem date_time REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenSSL)
include_directories(SYSTEM ${OPENSSL_INCLUDE_DIR})

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})

add_subdirectory(network)
add_subdirectory(scripting)
add_subdirectory(core)
add_subdirectory(plugins)
