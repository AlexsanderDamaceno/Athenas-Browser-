#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QScrollBar>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QDebug>
#include <algorithm>

#pragma once

const QStringList BLOCK_ELEMENTS = {

    "html", "body", "article", "section", "nav", "aside",
    "h1", "h2", "h3", "h4", "h5", "h6", "hgroup", "header",
    "footer", "address", "p", "hr", "pre", "blockquote",
    "ol", "ul", "menu", "li", "dl", "dt", "dd", "figure",
    "figcaption", "main", "div", "table", "form", "fieldset",
    "legend", "details", "summary"

};




class Element
{

public:
    Element() {}
    virtual ~Element() {};
    QList<Element*> children;
    Element* parent;
};

class TagElement : public Element
{
public:
    TagElement(const QString& tag, const QMap<QString, QString>& attributes, Element* parent)
        : tag(tag), attributes(attributes), parent(parent) {}
    QString tag;
    QMap<QString, QString> attributes;
    QList<Element*> children;
    Element* parent;
};

class TextElement : public Element
{

public:
    TextElement(const QString& text , Element *parent) : text(text), parent(parent) {}
    QString text;
    QList<Element*> children;
    Element* parent;
};

class ElementNode
{

public:
    ElementNode() {}
    virtual ~ElementNode() {};

    QList<ElementNode*> children;
    ElementNode* parent;
};


class TagNode : public ElementNode
{
public:
    TagNode(const QString& tag, const QMap<QString, QString>& attributes, ElementNode* parent)
        : tag(tag), attributes(attributes), parent(parent) {}
    QString tag;
    QMap<QString, QString> attributes;
    QList<ElementNode*> children;
    ElementNode* parent;
};

class TextNode : public ElementNode
{

public:
    TextNode(const QString& text , ElementNode *parent) : text(text), parent(parent) {}
    QString text;
    QList<ElementNode*> children;
    ElementNode* parent;
};
