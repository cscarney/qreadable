#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QQmlEngine>
#include <QTextStream>
#include <cstdio>
#include "jshelpers.h"
#include "dombuilder.h"
#include "domsupport.h"

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(readability);
    QCoreApplication app(argc, argv);

    auto urlString = QCoreApplication::arguments().value(1);
    auto url = QUrl(urlString);

    QNetworkAccessManager nam;
    QNetworkRequest req(url);
    QNetworkReply *reply = nam.get(req);
    QObject::connect(reply, &QNetworkReply::finished, &app, [reply]{
        QByteArray data = reply->readAll();
        QString text(data);

        QQmlEngine engine;
        QJSValue jsThis = engine.globalObject();
        engine.installExtensions(QJSEngine::ConsoleExtension);

        // create document
        DomBuilder builder(text);
        DomSupport::Document *document = builder.buildDocument(reply->url());
        QJSValue jsDocument = engine.newQObject(document);

        // load readability
        JSHelpers::evalFile(engine, ":/Readability.js");
        QJSValue readability = JSHelpers::callMemberConstructor(jsThis, "Readability", {jsDocument});

        // get result
        QJSValue parseResult = JSHelpers::callMember(readability, "parse");
        QJSValue content = parseResult.property("content");
        if (content.isString()) {
            QTextStream(stdout) << content.toString();
        }
        QCoreApplication::quit();
    });

    QCoreApplication::exec();
}
