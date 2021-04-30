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

#ifndef XMPPSOCKET_H
#define XMPPSOCKET_H

#include <QLocalServer>
#include <QObject>

#include "QXmppClient.h"

class XmppSocket : public QLocalServer
{
    Q_OBJECT
public:
    XmppSocket(QXmppClient *client, const QString &from, const QString &to);

private slots:
    void readyRead();

private:
    QXmppClient *client;
    QString from;
    QString to;

protected:
    void incomingConnection(quintptr socketDescriptor);
};

#endif // XMPPSOCKET_H
