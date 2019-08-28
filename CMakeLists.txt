cmake_minimum_required(VERSION 3.14)
project(WZQClient)

set(CMAKE_CXX_STANDARD 17)
AUX_SOURCE_DIRECTORY(. SRC_LIST)
link_libraries(ws2_32 wsock32)
add_executable(WZQClient ${SRC_LIST} UIUtils.cpp UIUtils.h config.h commonHeads.h ./JSONLib/cJSON.c ./JSONLib/cJSON.h ./JSONLib/CJsonObject.cpp ./JSONLib/CJsonObject.hpp Server.cpp Server.h)