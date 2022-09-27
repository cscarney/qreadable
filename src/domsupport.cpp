/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "domsupport.h"
#include <QQmlEngine>
#include <gumbo/gumbo.h>
#include <QPair>
#include "dombuilder.h"
using namespace QReadable;
using namespace QReadable::DomSupport;

void Node::clear()
{
    while (auto *last = lastChild()) {
        removeChild(last);
    }
}

DomSupport::Node *DomSupport::Node::firstChild() const
{
    return m_childNodes.value(0, nullptr);
}

DomSupport::Element *DomSupport::Node::firstElementChild() const
{
    return m_children.value(0, nullptr);
}

DomSupport::Node *DomSupport::Node::lastChild() const
{
    if (m_childNodes.isEmpty()) {
        return nullptr;
    }
    return m_childNodes.last();
}

DomSupport::Element *DomSupport::Node::lastElementChild() const
{
    if (m_children.isEmpty()) {
        return nullptr;
    }
    return m_children.last();
}

Document *Node::ownerDocument() const
{
    for(Node *eachAncestor=m_parentNode; eachAncestor; eachAncestor = eachAncestor->m_parentNode) {
        if (auto *doc = dynamic_cast<Document*>(eachAncestor)) {
            return doc;
        }
    }
    return nullptr;
}

static void walkNextElement(Element *&el) {
    if (Element *fec = el->firstElementChild()) {
        el = fec;
    } else if (Element *nes = el->m_nextElementSibling) {
        el = nes;
    } else {
        // find the first parent that has a next element
        while (el) {
            if (auto *parentElement = dynamic_cast<Element*>(el->m_parentNode)) {
                if (Element *parentNext = parentElement->m_nextElementSibling) {
                    el = parentNext;
                    return;
                }
                el = parentElement;
            } else {
                el = nullptr;
            }
        }
    }
}

QList<Element *> Node::getElementsByTagName(const QString &tag, int max_elems)
{
    int n{0};
    if (m_children.isEmpty()) {
        return {};
    }
    bool isGetAll = (tag=="*");
    QString tagUpper = tag.toUpper();
    GumboTag gumboTag = gumbo_tag_enum(tagUpper.toUtf8());
    QList<Element*> result;
    Element *candidate = m_children.first();
    while (candidate) {
        if (isGetAll || candidate->gumboTag() == gumboTag) {
            result.append(candidate);
            ++n;
            if (n>max_elems) {
                break;
            }
        }
        walkNextElement(candidate);
    }
    return result;
}

QList<Element *> Node::getElementsByTagName(const QString &tag)
{
    return getElementsByTagName(tag, INT_MAX);
}


void DomSupport::Node::appendChild(Node *child)
{
    child->setParent(this);
    if (child->m_parentNode) {
        child->m_parentNode->removeChild(child);
    }
    Node *last = lastChild();
    if (last) {
        last->m_nextSibling = child;
    }
    child->m_previousSibling = last;
    if (auto *childElement = dynamic_cast<Element*>(child)) {
        childElement->m_previousElementSibling = lastElementChild();
        m_children.push_back(childElement);
        if (Element *prev = childElement->m_previousElementSibling) {
            prev->m_nextElementSibling = childElement;
        }
    }
    m_childNodes.push_back(child);
    child->m_parentNode = this;
}

Node *Node::removeChild(Node *child)
{
    int childIndex = m_childNodes.indexOf(child);
    if (childIndex < 0) {
        return nullptr; // TODO should throw
    }
    child->m_parentNode = nullptr;
    Node *prev = child->m_previousSibling;
    Node *next = child->m_nextSibling;
    if (prev) {
        prev->m_nextSibling = next;
    }
    if (next) {
        next->m_previousSibling = prev;
    }

    if (auto childElement = dynamic_cast<Element*>(child)) {
        Element *prevElement = childElement->m_previousElementSibling;
        Element *nextElement = childElement->m_nextElementSibling;
        if (prevElement) {
            prevElement->m_nextElementSibling = nextElement;
        }
        if (nextElement) {
            nextElement->m_previousElementSibling = prevElement;
        }
        childElement->m_previousElementSibling = childElement->m_nextElementSibling = nullptr;
        m_children.removeOne(childElement);
    }
    child->m_previousSibling = child->m_nextSibling = nullptr;
    m_childNodes.removeAt(childIndex);
    return child;
}

static void updateElementLinks(Element *newElement, Element *oldElement)
{
    newElement->m_previousElementSibling = oldElement->m_previousElementSibling;
    newElement->m_nextElementSibling = oldElement->m_nextElementSibling;
    if (auto *prev = newElement->m_previousElementSibling) {
        prev->m_nextElementSibling = newElement;
    }
    if (auto *next = newElement->m_nextElementSibling) {
        next->m_previousElementSibling = newElement;
    }
}

static void updateElementLinks(Element *newElement, Node *oldNode)
{
    newElement->m_previousElementSibling = nullptr;
    for(auto *eachNode=oldNode->m_previousSibling; eachNode; eachNode=eachNode->m_previousSibling) {
        if (auto *eachElement = dynamic_cast<Element *>(eachNode)) {
            newElement->m_previousElementSibling = eachElement;
            break;
        }
    }
    if (auto *prev = newElement->m_previousElementSibling) {
        newElement->m_nextElementSibling = prev->m_nextElementSibling;
    } else {
        newElement->m_nextElementSibling = nullptr;
        for(auto *eachNode = oldNode->m_nextSibling; eachNode; eachNode=eachNode->m_nextSibling) {
            if (auto *eachElement = dynamic_cast<Element *>(eachNode)) {
                newElement->m_nextElementSibling = eachElement;
                break;
            }
        }
    }

    if (auto *prev = newElement->m_previousElementSibling) {
        prev->m_nextElementSibling = newElement;
    }
    if (auto *next = newElement->m_nextElementSibling) {
        next->m_previousElementSibling = newElement;
    }
}

static void updateElementLinks(Node */* newNode */, Element *oldElement) {
    if (auto *prev = oldElement->m_previousElementSibling) {
        prev->m_nextElementSibling = oldElement->m_nextElementSibling;
    }
    if (auto *next = oldElement->m_nextElementSibling) {
        next->m_previousElementSibling = oldElement->m_previousElementSibling;
    }
}

Node *Node::replaceChild(Node *newNode, Node *oldNode)
{
    int childIndex = m_childNodes.indexOf(oldNode);
    if (childIndex < 0) {
        return nullptr; // TODO should throw
    }
    newNode->setParent(this);
    if (Node *oldParent = newNode->m_parentNode) {
        oldParent->removeChild(newNode);
    }
    m_childNodes[childIndex] = newNode;
    newNode->m_nextSibling = oldNode->m_nextSibling;
    newNode->m_previousSibling = oldNode->m_previousSibling;
    if (Node *next = newNode->m_nextSibling) {
        next->m_previousSibling = newNode;
    }
    if (Node *prev = newNode->m_previousSibling) {
        prev->m_nextSibling = newNode;
    }
    newNode->m_parentNode = this;

    if (auto *newElement = dynamic_cast<Element*>(newNode)) {
        if (auto *oldElement = dynamic_cast<Element*>(oldNode)) {
            // replace an element with an element
            updateElementLinks(newElement, oldElement);
            m_children.replace(m_children.indexOf(oldElement), newElement);
        } else {
            // replace a non-element with an element
            updateElementLinks(newElement, oldNode);
            if (auto *next = newElement->m_nextElementSibling) {
                m_children.insert(m_children.indexOf(next), newElement);
            } else {
                m_children.push_back(newElement);
            }
        }
    } else if (auto *oldElement = dynamic_cast<Element*>(oldNode)) {
        // replace an element with a non-element
        updateElementLinks(newNode, oldElement);
        m_children.removeOne(oldElement);
    }

    oldNode->m_parentNode = nullptr;
    oldNode->m_previousSibling = nullptr;
    oldNode->m_nextSibling = nullptr;
    if (auto *oldElement = dynamic_cast<Element*>(oldNode)) {
        oldElement->m_previousElementSibling = nullptr;
        oldElement->m_nextElementSibling = nullptr;
    }
    return oldNode;
}

Attribute::Attribute(const QString &name, const QString &value)
    :m_name(name)
    ,m_value(value)
{}

void Attribute::serialize(QStringList &fragments, bool textOnly)
{
    if (textOnly) {
        return;
    }
    fragments << " " + m_name + "=\"" + getEncodedValue() + "\"";
}

QString Attribute::getEncodedValue() const
{
    return m_value.toHtmlEscaped();
}

QString Text::innerHTML()
{
    if (m_html.isNull()) {
        m_html = m_text.toHtmlEscaped();
    }
    return m_html;
}

void Text::setInnerHTML(const QString &html)
{
    m_html = html;
    m_text.clear();
}

QString Text::textContent()
{
    if (m_text.isNull()) {
        if (m_html.isEmpty()) {
            m_text = "";
        } else {
            QScopedPointer<Element> body(new Element("body"));
            DomBuilder builder(m_html, GUMBO_TAG_BODY);
            builder.buildIntoNode(body.data());
            m_text = body->textContent();
        }
    }
    return m_text;
}

void Text::setTextContent(const QString &text)
{
    m_text = text;
    m_html.clear();
}

void Text::appendTextContent(const QString &text, const QString &html)
{
    m_text += text;
    m_html += html;
}

void Text::serialize(QStringList &fragments, bool textOnly)
{
    if (textOnly) {
        fragments << textContent();
    } else {
        fragments << innerHTML();
    }
}

Document::Document(const QString& url)
    : m_url(url)
    , m_baseURI(url)
{}

void Document::serialize(QStringList &fragments, bool textOnly)
{
    for(auto *eachChild : qAsConst(m_childNodes)) {
        eachChild->serialize(fragments, textOnly);
    }
}

Element *Document::documentElement()
{
    return m_children.length() > 0 ? m_children.first() : nullptr;
}

QString Document::title()
{
    QList<Element*> match = getElementsByTagName("title", 1);
    if (match.isEmpty()){
        return QString();
    }
    return match.first()->textContent();
}

Element *Document::body()
{
    QList<Element*> match = getElementsByTagName("body");
    if (match.isEmpty()) {
        return nullptr;
    }
    return match.first();
}

Element *Document::head()
{
    QList<Element*> match = getElementsByTagName("head");
    if (match.isEmpty()) {
        return nullptr;
    }
    return match.first();
}

Element *Document::getElementById(const QString &id)
{
    if (m_children.isEmpty()) {
        return {};
    }
    Element *candidate = m_children.first();
    while (candidate) {
        if (candidate->id() == id) {
            return candidate;
        }
        walkNextElement(candidate);
    }
    return nullptr;
}

Element *Document::createElement(const QString &tag)
{
    return new Element(tag);
}

Text *Document::createTextNode(const QString &text)
{
    auto result = new Text();
    result->setTextContent(text);
    return result;
}


Element::Element(const QString &tag)
    : m_tagName(tag.toUpper())
    , m_style(new Style(this))
{}

Element::Element(int gumboTag)
    : Element(gumbo_normalized_tagname(static_cast<GumboTag>(gumboTag)))
{
    m_gumboTag = gumboTag;
}

void Element::serialize(QStringList &fragments, bool textOnly)
{
    if (textOnly) {
        serializeChildren(fragments, true);
        return;
    }
    QString name = localName();
    fragments << "<" << name;
    for(auto *eachAttr : qAsConst(m_attributes)) {
        eachAttr->serialize(fragments);
    }

    if (m_childNodes.isEmpty()) {
        fragments << "/>";
    } else {
        fragments << ">";
        serializeChildren(fragments, textOnly);
        fragments << "</" << name << ">";
    }
}

QString Element::innerHTML()
{
    QStringList fragments;
    serializeChildren(fragments, false);
    return fragments.join("");
}

void Element::setInnerHTML(const QString &html)
{
   clear();
   if (!html.isEmpty()) {
       DomBuilder builder(html, static_cast<GumboTag>(gumboTag()));
       builder.buildIntoNode(this);
   }
}

QString Element::textContent()
{
    QStringList fragments;
    serializeChildren(fragments, true);
    return fragments.join("");
}

void Element::setTextContent(const QString &text)
{
    clear();
    Text *textNode = new Text();
    textNode->setTextContent(text);
    appendChild(textNode);
}

QString Element::className() const
{
    return getAttribute("class");
}

void Element::setClassName(const QString &newClassName)
{
    setAttribute("class", newClassName);
}

QString Element::id() const
{
    return getAttribute("id");
}

void Element::setId(const QString &newId)
{
    setAttribute("id", newId);
}

QString Element::href() const
{
    return getAttribute("href");
}

void Element::setHref(const QString &newHref)
{
    setAttribute("href", newHref);
}

QString Element::src() const
{
    return getAttribute("src");
}

void Element::setSrc(const QString &newSrc)
{
    setAttribute("src", newSrc);
}

QString Element::srcset() const
{
    return getAttribute("srcset");
}

void Element::setSrcset(const QString &newSrcset)
{
    setAttribute("srcset", newSrcset);
}

QString Element::tagName() const
{
    return m_tagName;
}

QString Element::localName()
{
    return m_tagName.toLower();
}

int Element::gumboTag() const
{
    if (m_gumboTag < 0) {
        m_gumboTag = gumbo_tag_enum(m_tagName.toUtf8());
    }
    return m_gumboTag;
}

QString Element::getAttribute(const QString &name) const
{
    for (Attribute *eachAttr : qAsConst(m_attributes)) {
        if (eachAttr->m_name == name) {
            return eachAttr->m_value;
        }
    }
    return QString();
}

void Element::setAttribute(const QString &name, const QString &value)
{
    for (Attribute *eachAttr : qAsConst(m_attributes)) {
        if (eachAttr->m_name == name) {
            eachAttr->m_value = value;
            return;
        }
    }
    auto *newAttr = new Attribute(name, value);
    newAttr->setParent(this);
    m_attributes.append(newAttr);
}

void Element::removeAttribute(const QString &name)
{
    auto it = std::find_if(m_attributes.begin(), m_attributes.end(), [&name](Attribute *&attr){
        return name==attr->m_name;
    });
    if (it==m_attributes.end()) {
        return;
    }
    m_attributes.erase(it);
}

bool Element::hasAttribute(const QString &name)
{
    return std::find_if(m_attributes.begin(), m_attributes.end(), [&name](Attribute *&attr){
        return name==attr->m_name;
    })!=m_attributes.end();
}

void Element::serializeChildren(QStringList &fragments, bool textOnly)
{
    for(auto *eachChild : qAsConst(m_childNodes)) {
        eachChild->serialize(fragments, textOnly);
    }
}

Style::Style(Element *node)
    : QObject(node)
    , m_element(node)
{
}

QString Style::getStyle(const QString &styleName) const
{
    QString styleAttr = m_element->getAttribute("style");
    if (styleAttr.isEmpty()) {
        return QString();
    }
    const QStringList styles = styleAttr.split(";");
    for (const QString &css : styles) {
        QStringList splitCss = css.split(":");
        if (splitCss.length() != 2) {
            continue;
        }
        QString name = splitCss.first().trimmed();
        if (name==styleName) {
            return splitCss.last().trimmed();
        }
    }
    return QString();
}

void Style::setStyle(const QString &styleName, const QString &styleValue)
{
    QString cssText = m_element->getAttribute("style");
    const QString::iterator begin = cssText.begin();
    const QString::iterator end = cssText.end();
    QString::iterator index = begin;
    while (index < end) {
        QString::iterator declEnd = std::find(index, end, ';');
        QString::iterator colon = std::find(index, declEnd, ';');
        if (colon == declEnd){
            continue;
        }
        QStringView cssDecl(index, colon);
        if (cssDecl.trimmed()==styleName) {
            // found the entry for styleName, replace it
            QStringView prefix = QStringView(begin, index);
            QStringView suffix = declEnd < end ? QStringView(declEnd+1, end) : QStringView();
            cssText = QLatin1String("%1%2").arg(prefix, suffix);
            break;
        }
        index = declEnd + 1;
    }

    // didn't find
    if (!cssText.endsWith(';')) {
        cssText += ';';
    }
    cssText = QLatin1String("%1 %2: %3;").arg(cssText, styleName, styleValue);
    setProperty("style", cssText);
}

QString Style::display() const
{
    return getStyle("display");
}

void Style::setDisplay(const QString &newDisplay)
{
    setStyle("display", newDisplay);
}
