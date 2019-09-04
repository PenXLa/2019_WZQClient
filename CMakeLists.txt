cmake_minimum_required(VERSION 3.14)
project(WZQClient)

set(CMAKE_CXX_STANDARD 17)
AUX_SOURCE_DIRECTORY(. SRC_LIST)
link_libraries(ws2_32 wsock32)
set(CMAKE_EXE_LINKER_FLAGS -static)
add_executable(WZQClient ${SRC_LIST} UIUtils.cpp UIUtils.h config.h commonHeads.h ./JSONLib/cJSON.c ./JSONLib/cJSON.h ./JSONLib/CJsonObject.cpp ./JSONLib/CJsonObject.hpp Server.cpp Server.h config.cpp KeyEventHandler.cpp KeyEventHandler.h Game.cpp Game.h)