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

const int WIDTH = 800;
const int HEIGHT = 600;
const int HSTEP = 13;
const int VSTEP = 18;
const int SCROLL_STEP = 100;



struct DisplayItem

{
    DisplayItem() {

    }

    DisplayItem(int x , int y , QString word , QFont font): x(x) , y(y) , word(word) , font(font) {

    }
    int x;
    int y;
    QString word;
    QFont font;
};

class DrawCMD
{
public:
    DrawCMD()  {}

    virtual ~DrawCMD()  {}


    int top    = 0;
    int left   = 0;
    int right  = 0;
    int bottom = 0;

private:

};

class DrawText : public DrawCMD {
public:
    DrawText(int x1, int y1, const QString& text, const QFont& font)
        : top(y1), left(x1), text(text), font(font) {}

    int top;
    int left;
    QString text;
    QFont font;
private:

};

class DrawRect : public DrawCMD
{
public:
    DrawRect(int x1, int y1, int x2, int y2, const QColor& color)
        : top(y1), left(x1), bottom(y2), right(x2), color(color) {}

public:
    int top;
    int left;
    int bottom;
    int right;
    QColor color;
};
