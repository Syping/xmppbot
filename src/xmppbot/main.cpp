/*****************************************************************************
* xmppbot Simple unix socket based XMPP bot
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
#include <QFile>

#include "unixsocket.h"
#include "QXmppClient.h"
#include "QXmppMessage.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("xmppbot");
    app.setApplicationVersion("0.1");

    QCommandLineParser commandLineParser;
    commandLineParser.addPositionalArgument("config", QCoreApplication::translate("xmppbot", "Configuration file."));
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
            QTextStream(stderr) << "xmppbot: " << a_settingsPath << " not found!" << Qt::endl;
            return 1;
        }
    }

    QXmppClient client;

    bool loginSet = false;
    QString jid, jpw;
    QHash<QString, QString> h_msg;
    QHash<QString, QString> h_run;
    if (QFile::exists(settingsPath)) {
        QSettings settings(settingsPath, QSettings::IniFormat);
        for (const QString &group : settings.childGroups()) {
            settings.beginGroup(group);
            for (const QString &key : settings.childKeys()) {
                if (key == "Password") {
                    if (!loginSet) {
                        jid = group;
                        jpw = settings.value(key, QString()).toString();
                        loginSet = true;
                    }
                    else {
                        QTextStream(stderr) << "xmppbot: Login password can only be set once!" << Qt::endl;
                        return 1;
                    }
                }
                if (key == "UnixSocket") {
                    UnixSocket *unixSocket = new UnixSocket(&client, jid, group);
                    const QString permission = settings.value("SocketPermission", QString()).toString();
                    if (permission == "UG" || permission == "UserGroup") {
                        unixSocket->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption);
                    }
                    if (permission == "UO" || permission == "UserOther") {
                        unixSocket->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::OtherAccessOption);
                    }
                    if (permission == "U" || permission == "User") {
                        unixSocket->setSocketOptions(QLocalServer::UserAccessOption);
                    }
                    if (permission == "GO" || permission == "GroupOther") {
                        unixSocket->setSocketOptions(QLocalServer::GroupAccessOption | QLocalServer::OtherAccessOption);
                    }
                    if (permission == "G" || permission == "Group") {
                        unixSocket->setSocketOptions(QLocalServer::GroupAccessOption);
                    }
                    if (permission == "O" || permission == "Other") {
                        unixSocket->setSocketOptions(QLocalServer::OtherAccessOption);
                    }
                    if (permission == "A" || permission == "All" || permission == "UGO" || permission == "UserGroupOther") {
                        unixSocket->setSocketOptions(QLocalServer::WorldAccessOption);
                    }
                    const QString socketPath = settings.value(key, QString()).toString();
                    bool listen = unixSocket->listen(socketPath);
#ifdef Q_OS_UNIX
                    if (!listen) {
                        QLocalServer::removeServer(socketPath);
                        listen = unixSocket->listen(socketPath);
                    }
#endif
                    if (listen) {
                        QTextStream(stderr) << "xmppbot: Account socket " << group << " initialised" << Qt::endl;
                        const QString incoming = settings.value("Incoming", QString()).toString();
                        if (incoming.startsWith("message:")) {
                            QTextStream(stderr) << "xmppbot: Account message incoming " << group << " initialised" << Qt::endl;
                            h_msg.insert(group, incoming.mid(8));
                        }
                        if (incoming.startsWith("run:")) {
                            QTextStream(stderr) << "xmppbot: Account run incoming " << group << " initialised" << Qt::endl;
                            h_run.insert(group, incoming.mid(4));
                        }
                    }
                    else {
                        delete unixSocket;
                    }
                }
            }
            settings.endGroup();
        }
    }
    else {
        QTextStream(stderr) << "xmppbot: Can't initialise without settings.ini!" << Qt::endl;
        return 1;
    }

    if (jid.isEmpty() || jpw.isEmpty()) {
        QTextStream(stderr) << "xmppbot: Can't initialise without XMPP account!" << Qt::endl;
        return 1;
    }

    QObject::connect(&client, &QXmppClient::connected, [&]() {
        QTextStream(stderr) << "xmppbot: Account " << jid << " connected" << Qt::endl;
        QXmppPresence xmppPresence(QXmppPresence::Available);
        client.setClientPresence(xmppPresence);
    });

    QObject::connect(&client, &QXmppClient::disconnected, [&]() {
        QTextStream(stderr) << "xmppbot: Account " << jid << " disconnected" << Qt::endl;
        client.connectToServer(jid, jpw);
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
        const QString run = h_run.value(from_jid, QString());
        if (!run.isEmpty()) {
            qint64 pid;
            bool isStarted = QProcess::startDetached(run, QStringList() << from << xmppMessage.to() << xmppMessage.body(), QString(), &pid);
            if (isStarted) {
                QTextStream(stderr) << "xmppbot: Account " << from_jid << " executed pid " << pid << Qt::endl;
            }
        }
    });

    client.connectToServer(jid, jpw);

    QTextStream(stderr) << "xmppbot: Account login " << jid << " initialised" << Qt::endl;

    return app.exec();
}
