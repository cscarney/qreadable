/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "readable.h"
#include <QQmlEngine>
#include "dombuilder.h"
#include "jshelpers.h"
using namespace QReadable;

struct Readable::PrivData {
    QQmlEngine engine;
};

static void initResources()
{
    Q_INIT_RESOURCE(readability);
}

Readable::Readable(QObject *parent)
    : QObject(parent)
    , d{std::make_unique<PrivData>()}
{
    initResources();
    d->engine.installExtensions(QJSEngine::ConsoleExtension);
    JSHelpers::evalFile(d->engine, ":/Readability.js");
}

QString Readable::parse(const QString &htmlContent, const QUrl &url)
{
    DomBuilder builder(htmlContent);
    QScopedPointer<DomSupport::Document> document(builder.buildDocument(url));
    QJSValue jsDocument = d->engine.newQObject(document.get());

    QJSValue jsThis = d->engine.globalObject();
    QJSValue readability = JSHelpers::callMemberConstructor(jsThis, "Readability", {jsDocument});
    QJSValue parseResult = JSHelpers::callMember(readability, "parse");
    QJSValue content = parseResult.property("content");
    if (!content.isString()) {
        return QString();
    }
    return content.toString();
}

Readable::~Readable()=default;
