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

#include <QLocalSocket>

#include "xmppsocket.h"
#include "QXmppMessage.h"

XmppSocket::XmppSocket(QXmppClient *client, const QString &from, const QString &to) : client(client), from(from), to(to)
{
}

void XmppSocket::readyRead()
{
    QLocalSocket *localSocket = qobject_cast<QLocalSocket*>(sender());
    if (localSocket) {
        const QString message = QString::fromUtf8(localSocket->readAll());
        if (!message.isEmpty()) {
            QXmppMessage xmppMessage(from, to, message);
            client->sendPacket(xmppMessage);
        }
    }
}

void XmppSocket::incomingConnection(quintptr socketDescriptor)
{
    QLocalSocket *localSocket = new QLocalSocket(this);
    localSocket->setSocketDescriptor(socketDescriptor);
    QObject::connect(localSocket, &QLocalSocket::readyRead, this, &XmppSocket::readyRead);
    QObject::connect(localSocket, &QLocalSocket::disconnected, localSocket, &QLocalSocket::deleteLater);
}
