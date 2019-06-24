cmake_minimum_required(VERSION 3.4.1)
project(ruko_cpp)

set(CMAKE_CXX_STANDARD 14)

include_directories(source)
include_directories(libs)

add_library(ruko-common
        source/IndexManager.cpp
        source/IndexManager.hpp
        source/IndexNode.cpp
        source/IndexNode.hpp
        source/KeyMapper.cpp
        source/KeyMapper.hpp
        source/Object.cpp
        source/Object.hpp
        source/renderer.cpp
        source/renderer.hpp
        source/RukoDb.cpp
        source/RukoDb.hpp
        source/RukoServer.cpp
        source/RukoServer.hpp
        source/SaveScheduler.cpp
        source/SaveScheduler.hpp
        source/serialization.cpp
        source/serialization.hpp
        source/Socket.cpp
        source/Socket.hpp
        source/SocketServer.cpp
        source/SocketServer.hpp
        source/StarIndex.cpp
        source/StarIndex.hpp
        source/utils.cpp
        source/utils.hpp libs/backtrace.cpp libs/backtrace.hpp source/Logger.cpp)

add_executable(ruko-server
        source/main.cpp)

add_executable(ruko-test
        tests/catch.cpp
        tests/testIndexManager.cpp
        tests/testObject.cpp
        tests/testRuko.cpp
        tests/testSerialize.cpp)

target_link_libraries(ruko-common pthread)
target_link_libraries(ruko-common dl)
target_link_libraries(ruko-server ruko-common)
target_link_libraries(ruko-test ruko-common)

include(GNUInstallDirs)
install(TARGETS ruko-server DESTINATION ${CMAKE_INSTALL_BINDIR})
