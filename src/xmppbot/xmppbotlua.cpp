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

#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QTextStream>

#include "xmppbot.h"
#include "xmppbotlua.h"
#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"

XmppBotLua::XmppBotLua(QObject *parent) : QObject(parent)
{
    L = luaL_newstate();
    luaL_openlibs(L);

    // XMPP Functions
    pushFunction("jid", jid);
    pushFunction("jin", jin);
    pushFunction("sendMessage", sendMessage);
    pushFunction("setClientPresence", setClientPresence);

    // XMPP Presence
    pushVariant("PresenceAvailable", static_cast<int>(QXmppPresence::Available));
    pushVariant("PresenceSubscribe", static_cast<int>(QXmppPresence::Subscribe));
    pushVariant("PresenceSubscribed", static_cast<int>(QXmppPresence::Subscribed));
    pushVariant("PresenceUnavailable", static_cast<int>(QXmppPresence::Unavailable));
    pushVariant("PresenceUnsubscribe", static_cast<int>(QXmppPresence::Unsubscribe));
    pushVariant("PresenceUnsubscribed", static_cast<int>(QXmppPresence::Unsubscribed));
    pushVariant("StatusOnline", static_cast<int>(QXmppPresence::Online));
    pushVariant("StatusAway", static_cast<int>(QXmppPresence::Away));
    pushVariant("StatusSnooze", static_cast<int>(QXmppPresence::XA));
    pushVariant("StatusBusy", static_cast<int>(QXmppPresence::DND));
    pushVariant("StatusChat", static_cast<int>(QXmppPresence::Chat));

    // JSON
    pushFunction("jsonToTable", jsonToTable);
    pushFunction("tableToJson", tableToJson);
    pushVariant("JsonCompact", static_cast<int>(QJsonDocument::Compact));
    pushVariant("JsonIndented", static_cast<int>(QJsonDocument::Indented));

    // Process
    pushFunction("executeProcess", executeProcess);

    // Table
    pushFunction("tableContains", tableContains);
}

XmppBotLua::~XmppBotLua()
{
    lua_close(L);
}

lua_State* XmppBotLua::luaState()
{
    return L;
}

bool XmppBotLua::loadLuaScript(const QByteArray &data)
{
    const int result = luaL_loadbuffer(L, data.data(), data.size(), "script");
    return (result == 0);
}

bool XmppBotLua::executeLuaScript(const QByteArray &data)
{
    if (loadLuaScript(data))
        return (lua_pcall(L, 0, LUA_MULTRET, 0) == 0);
    return false;
}

bool XmppBotLua::executeLuaFunction(const char *name, bool requireReturn)
{
    return executeLuaFunction(L, name, requireReturn);
}

bool XmppBotLua::executeLuaFunction(lua_State *L_p, const char *name, bool requireReturn)
{
    const int returnCount = (requireReturn) ? LUA_MULTRET : 0;
    lua_getglobal(L_p, name);
    return (lua_pcall(L_p, 0, returnCount, 0) == 0);
}

bool XmppBotLua::executeLuaFunction(const char *name, const QVariant &argument, bool requireReturn)
{
    return executeLuaFunction(L, name, argument, requireReturn);
}

bool XmppBotLua::executeLuaFunction(lua_State *L_p, const char *name, const QVariant &argument, bool requireReturn)
{
    const int returnCount = (requireReturn) ? LUA_MULTRET : 0;
    lua_getglobal(L_p, name);
    pushVariant(L_p, argument);
    return (lua_pcall(L_p, 1, returnCount, 0) == 0);
}

bool XmppBotLua::executeLuaFunction(const char *name, const QVariantList &args, bool requireReturn)
{
    return executeLuaFunction(L, name, args, requireReturn);
}

bool XmppBotLua::executeLuaFunction(lua_State *L_p, const char *name, const QVariantList &args, bool requireReturn)
{
    int returnCount = (requireReturn) ? LUA_MULTRET : 0;
    lua_getglobal(L_p, name);
    for (const QVariant &argument : args) {
        pushVariant(L_p, argument);
    }
    return (lua_pcall(L_p, args.count(), returnCount, 0) == 0);
}

void XmppBotLua::pushFunction(const char *name, lua_CFunction function)
{
    pushFunction(L, name, function);
}

void XmppBotLua::pushFunction(lua_State *L_p, const char *name, lua_CFunction function)
{
    lua_pushcfunction(L_p, function);
    lua_setglobal(L_p, name);
}

void XmppBotLua::pushPointer(const char *name, void *pointer)
{
    pushPointer(L, name, pointer);
}

void XmppBotLua::pushPointer(lua_State *L_p, const char *name, void *pointer)
{
    pushPointer(L_p, pointer);
    lua_setglobal(L_p, name);
}

void XmppBotLua::pushPointer(void *pointer)
{
    pushPointer(L, pointer);
}

void XmppBotLua::pushPointer(lua_State *L_p, void *pointer)
{
    lua_pushlightuserdata(L_p, pointer);
}

void XmppBotLua::pushVariant(const char *name, const QVariant &variant)
{
    pushVariant(L, name, variant);
}

void XmppBotLua::pushVariant(lua_State *L_p, const char *name, const QVariant &variant)
{
    pushVariant(L_p, variant);
    lua_setglobal(L_p, name);
}

void XmppBotLua::pushVariant(const QVariant &variant)
{
    pushVariant(L, variant);
}

void XmppBotLua::pushVariant(lua_State *L_p, const QVariant &variant)
{
    if (variant.type() == QVariant::Bool) {
        lua_pushboolean(L_p, static_cast<int>(variant.toBool()));
    }
    else if (variant.type() == QVariant::Int) {
        lua_pushinteger(L_p, variant.toInt());
    }
    else if (variant.type() == QVariant::Double) {
        lua_pushnumber(L_p, variant.toDouble());
    }
    else if (variant.type() == QVariant::String) {
        lua_pushstring(L_p, variant.toString().toUtf8().data());
    }
    else if (variant.type() == QVariant::StringList) {
        const QStringList stringList = variant.toStringList();
        lua_createtable(L_p, 0, stringList.count());
        int currentId = 1;
        for (const QString &string : stringList) {
            lua_pushinteger(L_p, currentId);
            lua_pushstring(L_p, string.toUtf8().data());
            lua_settable(L_p, -3);
            currentId++;
        }
    }
    else if (static_cast<QMetaType::Type>(variant.type()) == QMetaType::QVariantList) {
        const QVariantList variantList = variant.toList();
        lua_createtable(L_p, 0, variantList.count());
        int currentId = 1;
        for (const QVariant &variant : qAsConst(variantList)) {
            lua_pushinteger(L_p, currentId);
            pushVariant(L_p, variant);
            lua_settable(L_p, -3);
            currentId++;
        }
    }
    else if (static_cast<QMetaType::Type>(variant.type()) == QMetaType::QVariantMap) {
        const QVariantMap variantMap = variant.toMap();
        lua_createtable(L_p, 0, variantMap.count());
        for (auto it = variantMap.constBegin(); it != variantMap.constEnd(); it++) {
            lua_pushstring(L_p, it.key().toUtf8().data());
            pushVariant(L_p, it.value());
            lua_settable(L_p, -3);
        }
    }
    else if (static_cast<QMetaType::Type>(variant.type()) == QMetaType::Void || static_cast<QMetaType::Type>(variant.type()) == QMetaType::VoidStar) {
        lua_pushlightuserdata(L_p, variant.value<void*>());
    }
    else {
        lua_pushnil(L_p);
    }
}

QVariant XmppBotLua::getVariant(const char *name)
{
    lua_getglobal(L, name);
    return returnVariant();
}

QVariant XmppBotLua::getVariant(lua_State *L_p, const char *name)
{
    lua_getglobal(L_p, name);
    return returnVariant(L_p);
}

QVariant XmppBotLua::getVariant(int index)
{
    return getVariant(L, index);
}

QVariant XmppBotLua::getVariant(lua_State *L_p, int index)
{
    if (lua_isboolean(L_p, index)) {
        return QVariant::fromValue(static_cast<bool>(lua_toboolean(L_p, index)));
    }
    else if (lua_isinteger(L_p, index)) {
        return QVariant::fromValue(lua_tointeger(L_p, index));
    }
    else if (lua_isnumber(L_p, index)) {
        return QVariant::fromValue(lua_tonumber(L_p, index));
    }
    else if (lua_isstring(L_p, index)) {
        return QVariant::fromValue(QString(lua_tostring(L_p, index)));
    }
    else if (lua_istable(L_p, index)) {
        QVariantMap variantMap;
        lua_pushvalue(L_p, index);
        lua_pushnil(L_p);
        while (lua_next(L_p, -2) != 0) {
            lua_pushvalue(L_p, -2);
            const QString key = QString(lua_tostring(L_p, -1));
            const QVariant value = getVariant(L_p, -2);
            variantMap.insert(key, value);
            lua_pop(L_p, 2);
        }
        lua_pop(L_p, 1);
        return QVariant::fromValue(variantMap);
    }
    else if (lua_isuserdata(L_p, index)) {
        return QVariant::fromValue(lua_touserdata(L_p, index));
    }
    else if (lua_isnoneornil(L_p, index)) {
        return QVariant();
    }
    QTextStream(stderr) << QLatin1String("Warning: Didn't catch lua_isnoneornil before empty QVariant got returned") << xendl;
    return QVariant();
}

void* XmppBotLua::returnPointer()
{
    return returnPointer(L);
}

void* XmppBotLua::returnPointer(lua_State *L_p)
{
    return getPointer(L_p, -1);
}

void* XmppBotLua::getPointer(const char *name)
{
    lua_getglobal(L, name);
    return returnPointer();
}

void* XmppBotLua::getPointer(lua_State *L_p, const char *name)
{
    lua_getglobal(L_p, name);
    return returnPointer(L_p);
}

void* XmppBotLua::getPointer(int index)
{
    return getPointer(L, index);
}

void* XmppBotLua::getPointer(lua_State *L_p, int index)
{
    return lua_touserdata(L_p, index);
}

QVariant XmppBotLua::returnVariant()
{
    return returnVariant(L);
}

QVariant XmppBotLua::returnVariant(lua_State *L_p)
{
    return getVariant(L_p, -1);
}

QVariantList XmppBotLua::getArguments(lua_State *L_p)
{
    QVariantList arguments;
    int argumentCount = getArgumentCount(L_p);
    for (int i = 1; i < (argumentCount + 1); i++) {
        arguments << getVariant(L_p, i);
    }
    return arguments;
}

int XmppBotLua::getArgumentCount(lua_State *L_p)
{
    return lua_gettop(L_p);
}

int XmppBotLua::jid(lua_State *L_p)
{
    QString jid;
    if (getArgumentCount(L_p) == 1) {
        const QString string = getVariant(L_p, 1).toString();
        for (const QChar &val : string) {
            if (val == '/')
                break;
            jid += val;
        }
    }
    pushVariant(L_p, jid);
    return 1;
}

int XmppBotLua::jin(lua_State *L_p)
{
    QString jin;
    if (getArgumentCount(L_p) == 1) {
        bool atJin = false;
        const QString string = getVariant(L_p, 1).toString();
        for (const QChar &val : string) {
            if (atJin)
                jin += val;
            else if (val == '/')
                atJin = true;
        }
    }
    pushVariant(L_p, jin);
    return 1;
}

int XmppBotLua::sendMessage(lua_State *L_p)
{
    bool packetSent = false;
    if (getArgumentCount(L_p) == 2) {
        const QString to = getVariant(L_p, 1).toString();
        const QString body = getVariant(L_p, 2).toString();
        QXmppMessage xmppMessage(QString(), to, body);
        packetSent = getXmppClient()->sendPacket(xmppMessage);
    }
    pushVariant(L_p, packetSent);
    return 1;
}

int XmppBotLua::setClientPresence(lua_State *L_p)
{
    bool presenceSet = false;
    if (getArgumentCount(L_p) >= 2 && getArgumentCount(L_p) <= 3) {
        const int presenceType = getVariant(L_p, 1).toInt();
        const int availableStatusType = getVariant(L_p, 2).toInt();
        QXmppPresence xmppPresence;
        xmppPresence.setType(static_cast<QXmppPresence::Type>(presenceType));
        xmppPresence.setAvailableStatusType(static_cast<QXmppPresence::AvailableStatusType>(availableStatusType));
        if (getArgumentCount(L_p) >= 3) {
            const QString statusText = getVariant(L_p, 3).toString();
            xmppPresence.setStatusText(statusText);
        }
        getXmppClient()->setClientPresence(xmppPresence);
        presenceSet = true;
    }
    pushVariant(L_p, presenceSet);
    return 1;
}

int XmppBotLua::jsonToTable(lua_State *L_p)
{
    if (getArgumentCount(L_p) >= 1) {
        const QJsonDocument jsonDocument = QJsonDocument::fromJson(getVariant(L_p, 1).toString().toUtf8());
        if (jsonDocument.isObject()) {
            pushVariant(L_p, jsonDocument.object().toVariantMap());
            return 1;
        }
        else if (jsonDocument.isArray()) {
            pushVariant(L_p, jsonDocument.array().toVariantList());
            return 1;
        }
    }
    return 0;
}

int XmppBotLua::tableToJson(lua_State *L_p)
{
    if (getArgumentCount(L_p) >= 1) {
        QJsonDocument::JsonFormat jsonFormat = QJsonDocument::Compact;
        if (getArgumentCount(L_p) >= 2) {
            jsonFormat = static_cast<QJsonDocument::JsonFormat>(getVariant(L_p, 2).toInt());
        }
        pushVariant(L_p, QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(getVariant(L_p, 1).toMap())).toJson(jsonFormat)));
        return 1;
    }
    return 0;
}

int XmppBotLua::executeProcess(lua_State *L_p)
{
    if (getArgumentCount(L_p) >= 1) {
        int processReturn = 0;
        bool runInBackground = false;
        bool processSuccessed = false;
        if (getArgumentCount(L_p) >= 2) {
            QStringList processArguments;
            QString processPath = getVariant(L_p, 1).toString();
            QVariant argument = getVariant(L_p, 2);
            if (static_cast<QMetaType::Type>(argument.type()) == QMetaType::QVariantMap) {
                const QVariantMap argumentMap = argument.toMap();
                for (auto it = argumentMap.constBegin(); it != argumentMap.constEnd(); it++) {
                    processArguments << it.value().toString();
                }
            }
            else if (argument.type() == QVariant::Bool) {
                runInBackground = argument.toBool();
            }
            else {
                processArguments << argument.toString();
            }
            if (getArgumentCount(L_p) >= 3) {
                if (argument.type() == QVariant::Bool) {
                    processArguments << argument.toString();
                }
                runInBackground = getVariant(L_p, 3).toBool();
            }
            if (runInBackground) {
                processSuccessed = QProcess::startDetached(processPath, processArguments);
            }
            else {
                processReturn = QProcess::execute(processPath, processArguments);
            }
        }
        else {
#if QT_VERSION >= 0x050F00
            processReturn = system(getVariant(L_p, 1).toString().toUtf8().constData());
#else
            processReturn = QProcess::execute(getVariant(L_p, 1).toString());
#endif
        }
        if (runInBackground && !processSuccessed) {
            processReturn = -2;
        }
        else if (!runInBackground && processReturn == 0) {
            processSuccessed = true;
        }
        pushVariant(L_p, processSuccessed);
        pushVariant(L_p, processReturn);
        return 2;
    }
    pushVariant(L_p, false);
    pushVariant(L_p, -2);
    return 2;
}

int XmppBotLua::tableContains(lua_State *L_p)
{
    if (getArgumentCount(L_p) >= 2) {
        const QVariant vtable = getVariant(L_p, 1);
        if (static_cast<QMetaType::Type>(vtable.type()) == QMetaType::QVariantMap) {
            const QVariantMap table = vtable.toMap();
            const QVariant toMatch = getVariant(L_p, 2);
            for (const QVariant &value : table) {
                if (value == toMatch) {
                    pushVariant(L_p, true);
                    return 1;
                }
            }
            pushVariant(L_p, false);
            return 1;
        }
    }
    return 0;
}
