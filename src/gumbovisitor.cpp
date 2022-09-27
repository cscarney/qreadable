/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: Apache-2.0
 */
#include "gumbovisitor.h"
using namespace QReadable;

GumboVisitor::GumboVisitor(const QString &input)
    : GumboVisitor(input.toUtf8())
{
}

GumboVisitor::GumboVisitor(const QByteArray &utf8Data)
    : GumboVisitor(gumbo_parse(utf8Data), utf8Data)
{
}

GumboVisitor::GumboVisitor(const QString &input, GumboTag fragmentContext)
    : GumboVisitor(input.toUtf8(), fragmentContext)
{
}

GumboVisitor::GumboVisitor(const QByteArray &utf8data, GumboTag fragmentContext)
    : GumboVisitor(gumbo_parse_fragment(&kGumboDefaultOptions, utf8data.data(), utf8data.length(), fragmentContext, GUMBO_NAMESPACE_HTML), utf8data)
{
}

GumboVisitor::GumboVisitor(GumboOutput *gumbo, const QByteArray &utf8data)
    : m_data{utf8data}
    , m_gumbo{gumbo}
    , m_root{m_gumbo->root}
    , m_node{m_root}
{
}

void GumboVisitor::walk()
{
    while (m_node != nullptr) {
        switch (m_node->type) {
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_CDATA:
        case GUMBO_NODE_WHITESPACE:
            visitText(m_node);
            break;
        case GUMBO_NODE_ELEMENT: {
            visitElementOpen(m_node);
            GumboElement &element = m_node->v.element;
            if (element.children.length > 0) {
                m_node = static_cast<GumboNode *>(element.children.data[0]);
                continue;
            }
            visitElementClose(m_node);
            break;
        }
        default:
            break;
        }
        moveNext();
    };
    finished();
}

void GumboVisitor::moveNext()
{
    do {
        unsigned int nextIndex = m_node->index_within_parent + 1;
        GumboElement &parent = m_node->parent->v.element;
        if (parent.children.length > nextIndex) {
            m_node = static_cast<GumboNode *>(parent.children.data[nextIndex]);
            return;
        }
        visitElementClose(m_node->parent);
        m_node = m_node->parent;
    } while (m_node != m_root);
    m_node = nullptr;
}

GumboVisitor::~GumboVisitor()
{
    gumbo_destroy_output(m_gumbo);
}
