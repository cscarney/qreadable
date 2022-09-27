/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include "gumbovisitor.h"
#include "domsupport.h"
#include <memory>
class QString;
class QJSValue;
class QUrl;

namespace QReadable {
/**
 * Builds a DOM document
 *
 * \sa DomSupport::Document
 */
class DomBuilder : public GumboVisitor
{
public:
    /**
     * Initialize a DomBuilder by parsing an entire HTML document
     */
    explicit DomBuilder(const QString &text);

    /**
     * Initialize a DomBuilder by parsing an HTML fragment
     */
    DomBuilder(const QString &text, GumboTag fragmentContext);
    ~DomBuilder();

    /**
     * Returns a new document node built from the parse tree.
     *
     * The caller assumes ownership of the returned document node.
     */
    DomSupport::Document *buildDocument(const QUrl &url);

    /**
     * Builds a parse tree with an existing node as the root element
     *
     * The root of the parse tree is associated with \a node and new
     * child nodes are appended into it. The existing children of \a node
     * are not modified or removed.
     */
    void buildIntoNode(DomSupport::Node *node);

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;

    void visitElementOpen(GumboNode *node) override;
    void visitText(GumboNode *node) override;
    void visitElementClose(GumboNode *node) override;
};
}
