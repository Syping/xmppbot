/*****************************************************************************
* xmppbotlua Lua Module for xmppbot
* Copyright (C) 2021 Syping
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* This software is provided as-is, no warranties are given to you, we are not
* responsible for anything with use of the software, you are self responsible.
*****************************************************************************/

#ifndef XMPPBOTLUA_H
#define XMPPBOTLUA_H

#include <QVariantList>
#include <QByteArray>
#include <QIODevice>
#include <QVariant>
#include <QString>
#include <QObject>
extern "C" {
#include "lua_module.h"
}

class XmppBotLua : public QObject
{
    Q_OBJECT
public:
    XmppBotLua(QObject *parent = nullptr);
    ~XmppBotLua();
    lua_State* luaState();
    bool loadLuaScript(const QByteArray &data);
    bool executeLuaScript(const QByteArray &data);
    bool executeLuaFunction(const char *name, bool requireReturn = false);
    static bool executeLuaFunction(lua_State *L_p, const char *name, bool requireReturn = false);
    bool executeLuaFunction(const char *name, const QVariant &argument, bool requireReturn = false);
    static bool executeLuaFunction(lua_State *L_p, const char *name, const QVariant &argument, bool requireReturn = false);
    bool executeLuaFunction(const char *name, const QVariantList &args, bool requireReturn = false);
    static bool executeLuaFunction(lua_State *L_p, const char *name, const QVariantList &args, bool requireReturn = false);
    void pushFunction(const char *name, lua_CFunction function);
    static void pushFunction(lua_State *L_p, const char *name, lua_CFunction function);
    void pushPointer(const char *name, void *pointer);
    static void pushPointer(lua_State *L_p, const char *name, void *pointer);
    void pushPointer(void *pointer);
    static void pushPointer(lua_State *L_p, void *pointer);
    void pushVariant(const char *name, const QVariant &variant);
    static void pushVariant(lua_State *L_p, const char *name, const QVariant &variant);
    void pushVariant(const QVariant &variant);
    static void pushVariant(lua_State *L_p, const QVariant &variant);
    QVariant getVariant(const char *name);
    static QVariant getVariant(lua_State *L_p, const char *name);
    QVariant getVariant(int index);
    static QVariant getVariant(lua_State *L_p, int index);
    void* returnPointer();
    static void* returnPointer(lua_State *L_p);
    void* getPointer(const char* name);
    static void* getPointer(lua_State *L_p, const char* name);
    void* getPointer(int index);
    static void* getPointer(lua_State *L_p, int index);
    QVariant returnVariant();
    static QVariant returnVariant(lua_State *L_p);
    static QVariantList getArguments(lua_State *L_p);
    static int getArgumentCount(lua_State *L_p);

private:
    // XMPP
    static int jid(lua_State *L_p);
    static int jin(lua_State *L_p);
    static int sendMessage(lua_State *L_p);
    static int setClientPresence(lua_State *L_p);

    // JSON
    static int jsonToTable(lua_State *L_p);
    static int tableToJson(lua_State *L_p);

    // Process
    static int executeProcess(lua_State *L_p);

    // Lua
    lua_State *L;
};

#endif // XMPPBOTLUA_H
