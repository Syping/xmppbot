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
#include <QEventLoop>
#include <QFile>

#include "xmppbotlua.h"
#include "xmppbotluathread.h"

XmppBotLuaThread::XmppBotLuaThread(const QString &filePath, const QString &lua_function, const QVariantList &lua_args, const bool &lua_globalthread) :
    filePath(filePath), lua_function(lua_function), lua_args(lua_args), lua_globalthread(lua_globalthread)
{
}

void XmppBotLuaThread::run()
{
    QByteArray script;
    if (QFile::exists(filePath)) {
        QFile scriptFile(filePath);
        if (scriptFile.open(QFile::ReadOnly)) {
            script = scriptFile.readAll();
            scriptFile.close();
        }
    }

    if (script.isEmpty())
        return;

    XmppBotLua xmppBotLua;
    xmppBotLua.executeLuaScript(script);
    xmppBotLua.executeLuaFunction(lua_function.toUtf8().constData(), lua_args);

    if (lua_globalthread) {
        QObject::connect(this, &XmppBotLuaThread::executeLuaFunction, this, [&](const QString &lua_function, const QVariantList &lua_args) {
            xmppBotLua.executeLuaFunction(lua_function.toUtf8().constData(), lua_args);
        });
        QEventLoop threadLoop;
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, &threadLoop, &QEventLoop::quit);
        threadLoop.exec();
    }
}
