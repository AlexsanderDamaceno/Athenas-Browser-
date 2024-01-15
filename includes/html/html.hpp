#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QScrollBar>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QDebug>
#include <algorithm>
#include "../elements/elements.hpp"



#pragma once

class HTMLParser {
public:
    HTMLParser(const QString& body) : body(body) {}



    QList<ElementNode*> parseTokens(const QList<Element *>& tokens)
    {


        QList<ElementNode*> currently_open;
        QList<ElementNode*> result;

        for (Element * tok : tokens)
        {

            ElementNode* parent = currently_open.isEmpty() ? nullptr : currently_open.last();

            implicitTags(tok, currently_open);
            if (dynamic_cast<TextElement *>(tok)) {
                const QString& text = dynamic_cast<TextElement *>(tok)->text;
                if (text.simplified().isEmpty()) continue;
                TextNode* node = new TextNode(text, parent);
                parent->children.append(node);
            } else if (dynamic_cast<TagElement *>(tok)->tag.startsWith("/")) {


                ElementNode* node = currently_open.last();
                currently_open.pop_back();

                if (currently_open.isEmpty()) {
                    result.append(node);
                } else {
                    currently_open.last()->children.append(node);
                }
            } else if (dynamic_cast<TagElement *>(tok) and SELF_CLOSING_TAGS.contains(dynamic_cast<TagElement *>(tok)->tag)) {

                TagElement *elem = dynamic_cast<TagElement *>(tok);
                ElementNode* node = new TagNode(elem->tag , elem->attributes  , parent);
                parent->children.append(node);

            } else if (dynamic_cast<TagElement *>(tok) and dynamic_cast<TagElement *>(tok)->tag.startsWith("!")) {
                continue;
            } else {
                ElementNode* node = new TagNode(dynamic_cast<TagElement *>(tok)->tag, dynamic_cast<TagElement *>(tok)->attributes , parent);
                currently_open.append(node);
            }
        }



        while (!currently_open.isEmpty()) {

            ElementNode* node = currently_open.last();
            currently_open.pop_back();

            if (currently_open.isEmpty()) {
                result.append(node);
            } else {
                currently_open.last()->children.append(node);
            }
        }



        return result;
    }

    QList<Element*> parse() {




        QString text;
        bool inTag = false;

        QList<Element *>  list;

        for (QChar c : body) {
            if (c == '<') {
                inTag = true;
                if (!text.isEmpty()) list.append(new TextElement(text , nullptr));
                text.clear();
            } else if (c == '>') {
                inTag = false;

                TagElement *tag =  addTag(text) ;
                list.append(tag);
                text.clear();
            } else {
                text += c;
            }
        }


        if (!inTag && !text.isEmpty()) {
            addText(text);
        }

       // qDebug() << "[PARSE FUNCTION FINISH]";
        return list;
    }

    QMap<QString, QString> getAttributes(const QString& text) {
        QStringList parts = text.split(' ');
        QString tag = parts[0].toLower();
        QMap<QString, QString> attributes;
        for (int i = 1; i < parts.size(); ++i) {
            QString attrPair = parts[i];
            if (attrPair.contains('=')) {
                QStringList keyAndValue = attrPair.split('=');
                QString key = keyAndValue[0].toLower();
                QString value = keyAndValue[1];
                if (value.size() > 2 && (value[0] == '\'' || value[0] == '\"')) {
                    value = value.mid(1, value.size() - 2);
                }
                attributes[key] = value;
            } else {
                attributes[attrPair.toLower()] = "";
            }
        }
        return attributes;
    }

    void addText(const QString& text) {
        if (text.simplified().isEmpty()) return;
       // implicitTags(nullptr , nul);
        Element* parent = unfinished.last();


        Element* node = new TextElement(text, parent);
        parent->children.append(node);
    }

    const QStringList SELF_CLOSING_TAGS = {
        "area", "base", "br", "col", "embed", "hr", "img", "input",
        "link", "meta", "param", "source", "track", "wbr"
    };

    TagElement *  addTag(const QString& tag)
    {

        QString tagLower = tag.toLower();
        QMap<QString, QString> attributes = getAttributes(tag);



        return new TagElement(tag, attributes , nullptr);

        /*
        if (tagLower.startsWith('!')) return;
        implicitTags(tagLower);
        if (tagLower.startsWith('/')) {
            if (unfinished.size() == 1) return;
            Element* node = unfinished.takeLast();
            Element* parent = unfinished.last();
            parent->children.append(node);
        } else if (SELF_CLOSING_TAGS.contains(tagLower)) {
            Element* parent = unfinished.last();
            Element* node = new TagElement(tagLower, attributes, parent);
            parent->children.append(node);
        } else {
            Element* parent = unfinished.isEmpty() ? nullptr : unfinished.last();
            Element* node = new TagElement(tagLower, attributes, parent);
            unfinished.append(node);
        }

*/
    }

    const QStringList HEAD_TAGS = {
        "base", "basefont", "bgsound", "noscript",
        "link", "meta", "title", "style", "script"
    };

    void implicitTags(Element * tok, QList<ElementNode*>& currently_open) {

        QString tag = "";
        if(dynamic_cast<TagElement*>(tok))
            tag =  dynamic_cast<TagElement*>(tok)->tag;



        int index = 0;


        while (true) {

            //qDebug() << "fdsaffds" << ++index;
            QStringList openTags;
            for (ElementNode* node : currently_open)
            {
                if(dynamic_cast<TagNode *>(node))
                    openTags.append(dynamic_cast<TagNode *>(node)->tag);
            }

            if (openTags.isEmpty() && tag != "html") {
                ElementNode* htmlNode = new TagNode("html", QMap<QString, QString>() , nullptr);
                currently_open.append(htmlNode);
            } else if (openTags == QStringList("html") && !QStringList("head body /html").contains(tag)) {
                QString implicit;
                if (HEAD_TAGS.contains(tag)) {
                    implicit = "head";
                } else {
                    implicit = "body";
                }
                ElementNode* parent = currently_open.last();
                ElementNode* implicitNode = new TagNode(implicit, QMap<QString, QString>() , parent);
                currently_open.append(implicitNode);
            } else if (openTags == QStringList("html head") &&
                       !QStringList("/head").contains(tag) && !HEAD_TAGS.contains(tag)) {
                ElementNode* node = currently_open.last();
                currently_open.pop_back();
                ElementNode* parent = currently_open.last();
                parent->children.append(node);
            } else {
                break;
            }
        }



    }

private:
    QString body;
    QList<Element*> unfinished;
};
