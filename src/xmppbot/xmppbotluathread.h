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

#ifndef XMPPBOTLUATHREAD_H
#define XMPPBOTLUATHREAD_H

#include <QThread>
#include <QObject>

class XmppBotLuaThread : public QThread
{
    Q_OBJECT
public:
    XmppBotLuaThread(const QString &filePath, const QString &lua_function, const QVariantList &lua_args, const bool &lua_globalthread = false);

protected:
    void run();

private:
    QString filePath;
    QString lua_function;
    QVariantList lua_args;
    bool lua_globalthread;

signals:
    void executeLuaFunction(const QString &lua_function, const QVariantList &lua_args);
};

#endif // XMPPBOTLUATHREAD_H
