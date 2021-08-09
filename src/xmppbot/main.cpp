/*****************************************************************************
* xmppbot Simple Unix Socket based XMPP bot
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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTextStream>
#include <QSettings>
#include <QProcess>
#include <QTimer>
#include <QFile>

#include "xmppbot.h"
#include "xmppbotlua.h"
#include "xmppbotluathread.h"
#include "xmppsocket.h"

#include "QXmppClient.h"
#include "QXmppMessage.h"
#include "QXmppPresence.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QLatin1String("xmppbot"));
    app.setApplicationVersion(QLatin1String("0.6.1"));

    QCommandLineParser commandLineParser;
    commandLineParser.addPositionalArgument(QLatin1String("config"), QCoreApplication::translate("xmppbot", "Configuration file."));
    commandLineParser.addHelpOption();
    commandLineParser.addVersionOption();
    commandLineParser.process(app);

    QString settingsPath = QLatin1String("settings.ini");
    const QStringList args = commandLineParser.positionalArguments();
    if (args.length() == 1) {
        const QString a_settingsPath = args.at(0);
        if (QFile::exists(a_settingsPath)) {
            settingsPath = a_settingsPath;
        }
        else {
            QTextStream(stderr) << QLatin1String("xmppbot: ") << a_settingsPath << QLatin1String(" not found!") << xendl;
            return 1;
        }
    }

    QXmppClient client;
    app.setProperty("XmppClient", QVariant::fromValue<QXmppClient*>(&client));

    bool loginSet = false;
    QString jid, jpw, script;
    QHash<QString, QString> h_msg;
    QHash<QString, QString> h_lua;
    QHash<QString, QString> h_run;
    if (QFile::exists(settingsPath)) {
        QSettings settings(settingsPath, QSettings::IniFormat);
        for (const QString &group : settings.childGroups()) {
            settings.beginGroup(group);
            if (group == QLatin1String("xmppbot")) {
                script = settings.value(QLatin1String("Script"), QString()).toString();
            }
            else {
                for (const QString &key : settings.childKeys()) {
                    if (key == QLatin1String("Password")) {
                        if (!loginSet) {
                            jid = group;
                            const QString instance = settings.value(QLatin1String("Instance"), QString()).toString();
                            if (!instance.isEmpty()) {
                                jid += QLatin1String("/") + instance;
                            }
                            jpw = settings.value(key, QString()).toString();
                            loginSet = true;
                        }
                        else {
                            QTextStream(stderr) << "xmppbot: Login password can only be set once!" << xendl;
                            return 1;
                        }
                    }
                    if (key == QLatin1String("Incoming")) {
                        const QString incoming = settings.value("Incoming", QString()).toString();
                        if (incoming.startsWith(QLatin1String("message:"))) {
                            QTextStream(stderr) << QLatin1String("xmppbot: Account message incoming ") << group << QLatin1String(" initialised") << xendl;
                            h_msg.insert(group, incoming.mid(8));
                        }
                        if (incoming.startsWith(QLatin1String("lua:"))) {
                            QTextStream(stderr) << QLatin1String("xmppbot: Account lua incoming ") << group << QLatin1String(" initialised") << xendl;
                            h_lua.insert(group, incoming.mid(4));
                        }
                        if (incoming.startsWith(QLatin1String("run:"))) {
                            QTextStream(stderr) << QLatin1String("xmppbot: Account run incoming ") << group << QLatin1String(" initialised") << xendl;
                            h_run.insert(group, incoming.mid(4));
                        }
                    }
                    if (key == QLatin1String(XmppSocketType)) {
                        XmppSocket *xmppSocket = new XmppSocket(&client, jid, group);
#ifdef Q_OS_UNIX
                        const QString permission = settings.value(QLatin1String("SocketPermission"), QString()).toString();
                        if (permission == QLatin1String("UG") || permission == QLatin1String("UserGroup")) {
                            xmppSocket->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption);
                        }
                        if (permission == QLatin1String("UO") || permission == QLatin1String("UserOther")) {
                            xmppSocket->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::OtherAccessOption);
                        }
                        if (permission == QLatin1String("U") || permission == QLatin1String("User")) {
                            xmppSocket->setSocketOptions(QLocalServer::UserAccessOption);
                        }
                        if (permission == QLatin1String("GO") || permission == QLatin1String("GroupOther")) {
                            xmppSocket->setSocketOptions(QLocalServer::GroupAccessOption | QLocalServer::OtherAccessOption);
                        }
                        if (permission == QLatin1String("G") || permission == QLatin1String("Group")) {
                            xmppSocket->setSocketOptions(QLocalServer::GroupAccessOption);
                        }
                        if (permission == QLatin1String("O") || permission == QLatin1String("Other")) {
                            xmppSocket->setSocketOptions(QLocalServer::OtherAccessOption);
                        }
                        if (permission == QLatin1String("A") || permission == QLatin1String("All") || permission == QLatin1String("UGO") || permission == QLatin1String("UserGroupOther")) {
                            xmppSocket->setSocketOptions(QLocalServer::WorldAccessOption);
                        }
#endif
                        const QString socketPath = settings.value(key, QString()).toString();
                        bool listen = xmppSocket->listen(socketPath);
#ifdef Q_OS_UNIX
                        if (!listen) {
                            QLocalServer::removeServer(socketPath);
                            listen = xmppSocket->listen(socketPath);
                        }
#endif
                        if (listen) {
                            QTextStream(stderr) << QLatin1String("xmppbot: Account socket ") << group << QLatin1String(" initialised") << xendl;
                        }
                        else {
                            delete xmppSocket;
                        }
                    }
                }
            }
            settings.endGroup();
        }
    }
    else {
        QTextStream(stderr) << QLatin1String("xmppbot: Can't initialise without settings.ini!") << xendl;
        return 1;
    }

    if (jid.isEmpty() || jpw.isEmpty()) {
        QTextStream(stderr) << QLatin1String("xmppbot: Can't initialise without XMPP account!") << xendl;
        return 1;
    }

    QTextStream(stderr) << QLatin1String("xmppbot: Account login ") << jid << QLatin1String(" initialised") << xendl;

    XmppBotLuaThread xmppBotLuaGlobalThread(script, QLatin1String("jidInitialised"), QVariantList() << jid, true);
    if (!script.isEmpty() && QFile::exists(script)) {
        xmppBotLuaGlobalThread.start();
    }

    QObject::connect(&client, &QXmppClient::stateChanged, [&](QXmppClient::State state) {
        switch (state) {
        case QXmppClient::ConnectedState: {
            QTextStream(stderr) << QLatin1String("xmppbot: Account ") << jid << QLatin1String(" connected") << xendl;
            QXmppPresence xmppPresence(QXmppPresence::Available);
            client.setClientPresence(xmppPresence);
            if (xmppBotLuaGlobalThread.isRunning()) {
                const QString lua_function = QLatin1String("jidConnected");
                const QVariantList lua_args = QVariantList() << jid;
                QMetaObject::invokeMethod(&xmppBotLuaGlobalThread, "executeLuaFunction", Qt::QueuedConnection, Q_ARG(QString, lua_function), Q_ARG(QVariantList, lua_args));
            }
            break;
        }
        case QXmppClient::ConnectingState:
            break;
        case QXmppClient::DisconnectedState:
            QTextStream(stderr) << QLatin1String("xmppbot: Account ") << jid << QLatin1String(" disconnected") << xendl;
            QTimer::singleShot(5000, &client, [&]() {
                client.connectToServer(jid, jpw);
            });
            if (xmppBotLuaGlobalThread.isRunning()) {
                const QString lua_function = QLatin1String("jidDisconnected");
                const QVariantList lua_args = QVariantList() << jid;
                QMetaObject::invokeMethod(&xmppBotLuaGlobalThread, "executeLuaFunction", Qt::QueuedConnection, Q_ARG(QString, lua_function), Q_ARG(QVariantList, lua_args));
            }
            break;
        default:
            break;
        }
    });

    QObject::connect(&client, &QXmppClient::messageReceived, [&](const QXmppMessage &xmppMessage) {
        const QString body = xmppMessage.body();
        if (body.isEmpty())
            return;
        const QString from = xmppMessage.from();
        QString from_jid;
        for (const QChar &val : from) {
            if (val == '/')
                break;
            from_jid += val;
        }
        const QString msg = h_msg.value(from_jid, QString());
        if (!msg.isEmpty()) {
            QXmppMessage xmppMessage(jid, from, msg);
            client.sendPacket(xmppMessage);
        }
        const QString lua = h_lua.value(from_jid, QString());
        if (!lua.isEmpty()) {
            XmppBotLuaThread *xmppBotLuaThread = new XmppBotLuaThread(lua, QLatin1String("messageReceived"), QVariantList() << from << xmppMessage.to() << xmppMessage.body());
            QObject::connect(xmppBotLuaThread, &XmppBotLuaThread::finished, xmppBotLuaThread, &XmppBotLuaThread::deleteLater);
            xmppBotLuaThread->start();
        }
        const QString run = h_run.value(from_jid, QString());
        if (!run.isEmpty()) {
            qint64 pid;
            bool isStarted = QProcess::startDetached(run, QStringList() << from << xmppMessage.to() << xmppMessage.body(), QString(), &pid);
            if (isStarted) {
                QTextStream(stderr) << QLatin1String("xmppbot: Account ") << from_jid << QLatin1String(" executed pid ") << pid << xendl;
            }
        }
        if (xmppBotLuaGlobalThread.isRunning()) {
            const QString lua_function = QLatin1String("messageReceived");
            const QVariantList lua_args = QVariantList() << from << xmppMessage.to() << xmppMessage.body();
            QMetaObject::invokeMethod(&xmppBotLuaGlobalThread, "executeLuaFunction", Qt::QueuedConnection, Q_ARG(QString, lua_function), Q_ARG(QVariantList, lua_args));
        }
    });

    QObject::connect(&client, &QXmppClient::presenceReceived, [&](const QXmppPresence &xmppPresence) {
        const QString from = xmppPresence.from();
        QString from_jid;
        for (const QChar &val : from) {
            if (val == '/')
                break;
            from_jid += val;
        }
        const QString lua = h_lua.value(from_jid, QString());
        if (!lua.isEmpty()) {
            XmppBotLuaThread *xmppBotLuaThread = new XmppBotLuaThread(lua, QLatin1String("presenceReceived"), QVariantList() << from << static_cast<int>(xmppPresence.type()) << static_cast<int>(xmppPresence.availableStatusType()) << xmppPresence.statusText());
            QObject::connect(xmppBotLuaThread, &XmppBotLuaThread::finished, xmppBotLuaThread, &XmppBotLuaThread::deleteLater);
            xmppBotLuaThread->start();
        }
        if (xmppBotLuaGlobalThread.isRunning()) {
            const QString lua_function = QLatin1String("presenceReceived");
            const QVariantList lua_args = QVariantList() << from << static_cast<int>(xmppPresence.type()) << static_cast<int>(xmppPresence.availableStatusType()) << xmppPresence.statusText();
            QMetaObject::invokeMethod(&xmppBotLuaGlobalThread, "executeLuaFunction", Qt::QueuedConnection, Q_ARG(QString, lua_function), Q_ARG(QVariantList, lua_args));
        }
    });

    client.connectToServer(jid, jpw);

    return app.exec();
}
