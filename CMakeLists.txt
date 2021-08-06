cmake_minimum_required(VERSION 3.7)

project(xmppbot VERSION 0.5 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Network REQUIRED)

include(cmake/QXmpp.cmake)
add_subdirectory(src/3rdparty/lua54)

add_executable(xmppbot
    src/xmppbot/main.cpp
    src/xmppbot/xmppbot.h
    src/xmppbot/xmppbotlua.cpp
    src/xmppbot/xmppbotlua.h
    src/xmppbot/xmppbotluathread.cpp
    src/xmppbot/xmppbotluathread.h
    src/xmppbot/xmppsocket.cpp
    src/xmppbot/xmppsocket.h
)
add_dependencies(xmppbot xmppbotlua ${QXmppLibrary})

target_link_libraries(xmppbot Qt${QT_VERSION_MAJOR}::Network xmppbotlua ${QXmppLibrary})

install(TARGETS xmppbot DESTINATION bin)
