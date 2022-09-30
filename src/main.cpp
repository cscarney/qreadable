/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextStream>
#include <cstdio>
#include "readable.h"


using namespace QReadable;
int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(readability);
    QCoreApplication app(argc, argv);

    auto urlString = QCoreApplication::arguments().value(1);
    auto url = QUrl(urlString);

    QNetworkAccessManager nam;
    QNetworkRequest req(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &app, [reply]{
        if (reply->error()!=QNetworkReply::NoError) {
            qWarning() << "Failed to load content: " << reply->errorString();
            QCoreApplication::exit(1);
        }
        QByteArray data = reply->readAll();
        QString text(data);
        Readable readable;
        QTextStream(stdout) << readable.parse(text, reply->url());
        QCoreApplication::quit();
    });

    QCoreApplication::exec();
}
