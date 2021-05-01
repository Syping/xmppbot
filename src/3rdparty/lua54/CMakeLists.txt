cmake_minimum_required(VERSION 3.7)

project(xmppbotlua VERSION 5.4 LANGUAGES C)

add_library(xmppbotlua STATIC
    lapi.c
    lauxlib.c
    lbaselib.c
    lcode.c
    lcorolib.c
    lctype.c
    ldblib.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    linit.c
    liolib.c
    llex.c
    lmathlib.c
    lmem.c
    loadlib.c
    lobject.c
    lopcodes.c
    loslib.c
    lparser.c
    lstate.c
    lstring.c
    lstrlib.c
    ltable.c
    ltablib.c
    ltm.c
    lundump.c
    lutf8lib.c
    lvm.c
    lzio.c
    lapi.h
    lauxlib.h
    lcode.h
    lctype.h
    ldebug.h
    ldo.h
    lfunc.h
    lgc.h
    ljumptab.h
    llex.h
    llimits.h
    lmem.h
    lobject.h
    lopcodes.h
    lopnames.h
    lparser.h
    lprefix.h
    lstate.h
    lstring.h
    ltable.h
    ltm.h
    lua_module.h
    lua.h
    luaconf.h
    lualib.h
    lundump.h
    lvm.h
    lzio.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_include_directories(xmppbotlua PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(xmppbotlua PUBLIC
    -DLUA_COMPAT_5_2
    -DLUA_USE_LINUX
)

target_link_libraries(xmppbotlua PUBLIC
    ${CMAKE_DL_LIBS}
)
