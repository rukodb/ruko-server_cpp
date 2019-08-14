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
        source/objects/Object.cpp
        source/objects/Object.hpp
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
        source/utils.hpp
        libs/backtrace.cpp
        libs/backtrace.hpp
        source/Logger.cpp

        source/objects/BoolData.cpp
        source/objects/BoolData.hpp
        source/objects/DictData.cpp
        source/objects/DictData.hpp
        source/objects/FloatData.cpp
        source/objects/FloatData.hpp
        source/objects/IndexableData.cpp
        source/objects/IndexableData.hpp
        source/objects/IntData.cpp
        source/objects/IntData.hpp
        source/objects/ListData.cpp
        source/objects/ListData.hpp
        source/objects/NullData.cpp
        source/objects/NullData.hpp
        source/objects/ObjData.cpp
        source/objects/ObjData.hpp
        source/objects/Object.cpp
        source/objects/Object.hpp
        source/objects/StringData.cpp
        source/objects/StringData.hpp

        source/commands/Command.cpp
        source/commands/Command.hpp
        source/commands/CreateMappingCommand.cpp
        source/commands/CreateMappingCommand.hpp
        source/commands/DeclareCommand.cpp
        source/commands/DeclareCommand.hpp
        source/commands/DeleteCommand.cpp
        source/commands/DeleteCommand.hpp
        source/commands/DeleteMappingCommand.cpp
        source/commands/DeleteMappingCommand.hpp
        source/commands/GetCommand.cpp
        source/commands/GetCommand.hpp
        source/commands/GetMappingsCommand.cpp
        source/commands/GetMappingsCommand.hpp
        source/commands/LputCommand.cpp
        source/commands/LputCommand.hpp
        source/commands/SetCommand.cpp
        source/commands/SetCommand.hpp
        source/commands/HelpCommand.cpp
        source/commands/HelpCommand.hpp
        source/commands/parsing.cpp
        source/commands/parsing.hpp)

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
