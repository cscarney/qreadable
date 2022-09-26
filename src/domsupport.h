#ifndef DOMSUPPORT_H
#define DOMSUPPORT_H
#include <QObject>
#include <QUrl>
#include <QVariant>

namespace DomSupport {
class Document;
class Attribute;
class Element;
class Style;

/**
 * Base class for all DOM nodes
 *
 * This class is designed to be used from JavaScript, and as such
 * does not provide a clean or stable C++ interface to its properties,
 * many of which simply expose member variables.
 *
 * Nodes take ownership of their added children, and keep them
 * alive until the root node of the tree is destroyed, even if
 * they are removed from the tree.  This accomodates JavaScript
 * code that may hold references to removed nodes.
 *
 * This does not implement the entire DOM spec, only the specific
 * functions used by Readability.js.
 */
class Node : public QObject {
    Q_OBJECT

public:
    enum NodeType {
        UNKNOWN_NODE = 0,
        ELEMENT_NODE = 1,
        ATTRIBUTE_NODE = 2,
        TEXT_NODE = 3,
        CDATA_SECTIONS_NODE = 4,
        ENTITY_REFERENCE_NODE = 5,
        ENTITY_NODE = 6,
        PROCESSING_INSTRUCTION_NODE = 7,
        COMMENT_NODE = 8,
        DOCUMENT_NODE = 9,
        DOCUMENT_TYPE_NODE = 10,
        DOCUMENT_FRAGMENT_NODE = 11,
        NOTATION_NODE = 12
    };
    Q_ENUM(NodeType)

    Q_PROPERTY(QList<DomSupport::Attribute*> attributes MEMBER m_attributes)
    Q_PROPERTY(QList<DomSupport::Node*> childNodes MEMBER m_childNodes)
    Q_PROPERTY(QList<DomSupport::Element*> children MEMBER m_children)
    Q_PROPERTY(QString localName MEMBER m_localName)
    Q_PROPERTY(DomSupport::Node *parentNode MEMBER m_parentNode)
    Q_PROPERTY(DomSupport::Node *previousSibling MEMBER m_previousSibling)
    Q_PROPERTY(DomSupport::Node *nextSibling MEMBER m_nextSibling)
    Q_PROPERTY(DomSupport::Node *firstChild READ firstChild)
    Q_PROPERTY(DomSupport::Element *firstElementChild READ firstElementChild)
    Q_PROPERTY(DomSupport::Node *lastChild READ lastChild)
    Q_PROPERTY(DomSupport::Element *lastElementChild READ lastElementChild)
    Q_PROPERTY(QString nodeName READ nodeName)
    Q_PROPERTY(DomSupport::Node::NodeType nodeType READ nodeType)
    Q_PROPERTY(DomSupport::Document *ownerDocument READ ownerDocument)

    virtual NodeType nodeType() { return UNKNOWN_NODE; }
    virtual QString nodeName() { return QString(); }
    virtual void serialize(QStringList &fragments, bool textOnly=false){}
    void clear();
    DomSupport::Node *firstChild() const;
    DomSupport::Element *firstElementChild() const;
    DomSupport::Node *lastChild() const;
    DomSupport::Element *lastElementChild() const;
    DomSupport::Document *ownerDocument() const;

    QList<DomSupport::Element*> getElementsByTagName(const QString &tag, int max_elems);
    Q_INVOKABLE QList<DomSupport::Element*> getElementsByTagName(const QString &tag);
    Q_INVOKABLE void appendChild(DomSupport::Node *child);
    Q_INVOKABLE DomSupport::Node *removeChild(DomSupport::Node *child);
    Q_INVOKABLE DomSupport::Node *replaceChild(DomSupport::Node *newNode, DomSupport::Node *oldNode);

    QList<DomSupport::Attribute *> m_attributes;
    QList<DomSupport::Node *> m_childNodes;
    QList<DomSupport::Element *> m_children;
    QString m_localName;
    QString m_nodeName;
    DomSupport::Node *m_parentNode{nullptr};
    DomSupport::Node *m_previousSibling{nullptr};
    DomSupport::Node *m_nextSibling{nullptr};
};

class Attribute : public Node {
    Q_OBJECT

public:
    Q_PROPERTY(QString name MEMBER m_name)
    Q_PROPERTY(QString value MEMBER m_value)

    Attribute(const QString &name, const QString &value);

    NodeType nodeType() override { return ATTRIBUTE_NODE; }
    void serialize(QStringList &fragments, bool textOnly=false) override;

    Q_INVOKABLE QString getEncodedValue() const;

    QString m_name;
    QString m_value;
};

class Comment : public Node {
    Q_OBJECT
public:
    NodeType nodeType() override { return COMMENT_NODE; }
    QString nodeName() override { return "#comment"; }
};

class AbstractContentNode : public Node {
    Q_OBJECT
public:
    Q_PROPERTY(QString innerHTML READ innerHTML WRITE setInnerHTML)
    Q_PROPERTY(QString textContent READ textContent WRITE setTextContent)

    virtual QString innerHTML()=0;
    virtual void setInnerHTML(const QString &html)=0;
    virtual QString textContent()=0;
    virtual void setTextContent(const QString &text)=0;
};


class Text : public AbstractContentNode {
    Q_OBJECT
public:
    NodeType nodeType() override { return TEXT_NODE; }
    QString nodeName() override { return "#text"; }
    QString innerHTML() override;
    void setInnerHTML(const QString &html) override;
    QString textContent() override;
    void setTextContent(const QString &text) override;
    void appendTextContent(const QString &text);
    void serialize(QStringList &fragments, bool textOnly=false) override;

private:
    QString m_html;
    QString m_text;
};

class Document : public Node {
    Q_OBJECT
public:
    Q_PROPERTY(QString documentURI MEMBER m_url)
    Q_PROPERTY(QUrl baseURI MEMBER m_baseURI)
    Q_PROPERTY(DomSupport::Element *documentElement READ documentElement)
    Q_PROPERTY(QString title READ title)
    Q_PROPERTY(DomSupport::Element *body READ body);
    Q_PROPERTY(DomSupport::Element *head READ head);

    explicit Document(const QString& url);
    NodeType nodeType() override { return DOCUMENT_NODE; }
    QString nodeName() override { return "#document"; }
    void serialize(QStringList &fragments, bool textOnly=false) override;
    Element *documentElement();
    QString title();
    Element *body();
    Element *head();

    Q_INVOKABLE Element *getElementById(const QString &id);
    Q_INVOKABLE DomSupport::Element *createElement(const QString &tag);
    Q_INVOKABLE DomSupport::Text *createTextNode(const QString &text);

    QString m_url;
    QUrl m_baseURI;
};

class Element : public AbstractContentNode {
    Q_OBJECT
public:
    Q_PROPERTY(QString tagName READ tagName)
    Q_PROPERTY(Element *previousElementSibling MEMBER m_previousElementSibling)
    Q_PROPERTY(Element *nextElementSibling MEMBER m_nextElementSibling)
    Q_PROPERTY(Style *style READ style)
    Q_PROPERTY(QString className READ className WRITE setClassName)
    Q_PROPERTY(QString id READ id WRITE setId)
    Q_PROPERTY(QString href READ href WRITE setHref)
    Q_PROPERTY(QString src READ src WRITE setSrc)
    Q_PROPERTY(QString srcset READ srcset WRITE setSrcset)
    Q_PROPERTY(QString localName READ localName)

    explicit Element(const QString &tag);
    explicit Element(int gumboTag);

    NodeType nodeType() override { return ELEMENT_NODE; }
    QString nodeName() override { return m_tagName; }
    void serialize(QStringList &fragments, bool textOnly=false) override;
    QString innerHTML() override;
    void setInnerHTML(const QString &html) override;
    QString textContent() override;
    void setTextContent(const QString &text) override;
    Style *style() const { return m_style; }
    QString className() const;
    void setClassName(const QString &newClassName);
    QString id() const;
    void setId(const QString &newId);
    QString href() const;
    void setHref(const QString &newHref);
    QString src() const;
    void setSrc(const QString &newSrc);
    QString srcset() const;
    void setSrcset(const QString &newSrcset);
    QString tagName() const;
    void setTagName(const QString &tagName);
    QString localName();
    int gumboTag() const;

    Q_INVOKABLE QString getAttribute(const QString &name) const;
    Q_INVOKABLE void setAttribute(const QString &name, const QString &value);
    Q_INVOKABLE void removeAttribute(const QString &name);
    Q_INVOKABLE bool hasAttribute(const QString &name);

    Element *m_previousElementSibling{nullptr};
    Element *m_nextElementSibling{nullptr};

private:
    QString m_tagName;
    mutable int m_gumboTag{-1};
    Style *m_style{nullptr};
    void serializeChildren(QStringList &fragments, bool textOnly);
};

class Style : public QObject {
    Q_OBJECT
public:
    Q_PROPERTY(QString display READ display WRITE setDisplay)

    explicit Style(Element *node);
    Q_INVOKABLE QString getStyle(const QString &styleName) const;
    Q_INVOKABLE void setStyle(const QString &styleName, const QString &styleValue);
    QString display() const;
    void setDisplay(const QString &newDisplay);

private:
    Element *m_element;
};
}

#endif
