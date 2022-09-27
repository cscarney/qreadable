/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <QObject>
#include <QUrl>
#include <memory>
#include "readable-defs.h"

namespace QReadable {
class QREADABLE_EXPORT Readable : public QObject
{
    Q_OBJECT
public:
    explicit Readable(QObject *parent = nullptr);
    ~Readable();
    QString parse(const QString &htmlContent, const QUrl &url=QUrl());

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
};
}
