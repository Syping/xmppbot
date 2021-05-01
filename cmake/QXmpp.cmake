cmake_minimum_required(VERSION 3.7)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Xml REQUIRED)

option(BUILD_SYSTEM_QXMPP "Build with system QXmpp Library" OFF)
if(BUILD_SYSTEM_QXMPP)
    find_package(QXmpp)
    if (QXmpp_FOUND)
        get_target_property(QXmppLinkLibraries QXmpp::QXmpp INTERFACE_LINK_LIBRARIES)
        if("${QXmppLinkLibraries}" MATCHES "Qt${QT_VERSION_MAJOR}")
            if(QXmpp_VERSION VERSION_LESS "1.4.0")
                set(CMAKE_CXX_STANDARD 11)
            endif()
            set(QXmppLibrary QXmpp::QXmpp)
        else()
            message(FATAL_ERROR "QXmpp Library incorrect Qt Version")
        endif()
    else()
        message(FATAL_ERROR "QXmpp Library not found")
    endif()
else()
    project(qxmpp VERSION 1.4.0)
    set(SO_VERSION 3)
    set(QXmppLibrary qxmpp)
    add_subdirectory(src/3rdparty/qxmpp/src)
endif()

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050F00
    -DQURL_NO_CAST_FROM_STRING
    -DQT_NO_CAST_TO_ASCII
    -DQT_NO_FOREACH
)
