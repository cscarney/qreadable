/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "gumbo/gumbo.h"
#include <QString>

namespace QReadable {
class GumboVisitor
{
public:
    explicit GumboVisitor(const QString &input);
    explicit GumboVisitor(const QByteArray &utf8data);
    GumboVisitor(const QString &input, GumboTag fragmentContext);
    GumboVisitor(const QByteArray &utf8data, GumboTag fragmentContext);
    explicit GumboVisitor(GumboOutput *gumbo, const QByteArray &utf8data=QByteArray());
    virtual ~GumboVisitor();
    GumboVisitor(GumboVisitor &other) = delete;
    void operator=(GumboVisitor &other) = delete;

    /**
     * Walk the element tree.
     *
     * This calls the appropriate visit* methods for each node in the parse tree
     */
    void walk();

    /**
     * The root node of the parse tree.
     */
    GumboNode *const &root()
    {
        return m_root;
    }

private:
    virtual void visitElementOpen(GumboNode *node){};
    virtual void visitText(GumboNode *node){};
    virtual void visitElementClose(GumboNode *node){};
    virtual void finished(){};

    const QByteArray m_data;
    GumboOutput *m_gumbo;
    GumboNode *m_root;
    GumboNode *m_node;
    void moveNext();
};
}
