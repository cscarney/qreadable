/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#include "dombuilder.h"
#include <QDebug>
#include <QJSValue>
#include <QStack>
#include <QUrl>
#include "domsupport.h"
using namespace QReadable;
using namespace QReadable::DomSupport;

struct DomBuilder::PrivData {
    Document *document{nullptr};
    Node *rootNode{nullptr};
    QStack<Element*> elementStack;
    GumboNode *skipNode{nullptr};
    Text *currentText{nullptr};
};

DomBuilder::~DomBuilder() = default;

DomBuilder::DomBuilder(const QString &text)
    : GumboVisitor(text)
    , d{std::make_unique<PrivData>()}
{
}

DomBuilder::DomBuilder(const QString &text, GumboTag fragmentContext)
    : GumboVisitor(text, fragmentContext)
    , d{std::make_unique<PrivData>()}
{}


void DomBuilder::buildIntoNode(Node *element)
{
    d->rootNode = element;
    d->document = dynamic_cast<Document*>(element);
    d->skipNode = d->document ? nullptr : root();
    walk();
}

Document *DomBuilder::buildDocument(const QUrl &url)
{
    d->rootNode = d->document = new Document(url.toString());
    walk();
    return d->document;
}

static QString getTagName(GumboStringPiece originalTag)
{
    gumbo_tag_from_original_text(&originalTag);
    QString tagName = QString::fromUtf8(originalTag.data, originalTag.length);
    return tagName.section(':', -1, -1);
}

void DomBuilder::visitElementOpen(GumboNode *node)
{   
    d->currentText = nullptr;
    if (node==d->skipNode) {
        return;
    }
    GumboElement &element = node->v.element;
    Element *domElement = element.tag==GUMBO_TAG_UNKNOWN ?
                new Element(getTagName(element.original_tag)) :
                new Element(element.tag);

    GumboVector attrs = element.attributes;
    for(unsigned int i=0; i<attrs.length; i++) {
        const GumboAttribute *attr = static_cast<GumboAttribute *>(attrs.data[i]);
        domElement->setAttribute(attr->name, attr->value);
    }

    if (d->elementStack.isEmpty()) {
        d->rootNode->appendChild(domElement);
    } else {
        Element *parent = d->elementStack.top();
        parent->appendChild(domElement);
    }

    if (d->document && element.tag == GUMBO_TAG_BASE) {
        GumboAttribute *href = gumbo_get_attribute(&element.attributes, "href");
        if (href) {
            d->document->m_baseURI = href->value;
        }
    }
    d->elementStack.push(domElement);
}

void DomBuilder::visitText(GumboNode *node)
{
    QString text = node->v.text.text;
    GumboStringPiece rawSource = node->v.text.original_text;
    auto source = QString::fromUtf8(rawSource.data, rawSource.length);
    if (!d->currentText) {
        d->currentText = new Text();
        if (d->elementStack.isEmpty()) {
            d->rootNode->appendChild(d->currentText);
        } else {
            Element *parent = d->elementStack.top();
            parent->appendChild(d->currentText);
        }
    }
    d->currentText->appendTextContent(text, source);
}

void DomBuilder::visitElementClose(GumboNode *node)
{
    d->currentText = nullptr;
    if (node == d->skipNode) {
        return;
    }
    d->elementStack.pop();
}
