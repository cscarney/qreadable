#include "dombuilder.h"
#include <QDebug>
#include <QJSValue>
#include <QStack>
#include <QUrl>
#include "domsupport.h"
using namespace DomSupport;

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

void DomBuilder::visitElementOpen(GumboNode *node)
{   
    d->currentText = nullptr;
    if (node==d->skipNode) {
        return;
    }
    GumboElement &element = node->v.element;
    auto *domElement = new Element(element.tag);
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

    if (Text *currentText = d->currentText) {
        currentText->appendTextContent(text);
        return;
    }
    Text *domText = new Text();
    domText->setTextContent(text);

    if (d->elementStack.isEmpty()) {
        d->rootNode->appendChild(domText);
    } else {
        Element *parent = d->elementStack.top();
        parent->appendChild(domText);
    }
}

void DomBuilder::visitElementClose(GumboNode *node)
{
    d->currentText = nullptr;
    if (node == d->skipNode) {
        return;
    }
    d->elementStack.pop();
}
