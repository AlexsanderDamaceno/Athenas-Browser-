#include "mainwindow.h"

#include <QApplication>
#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QDebug>

const int WIDTH = 800;
const int HEIGHT = 600;
const int HSTEP = 13;
const int VSTEP = 18;
const int SCROLL_STEP = 100;

class Base_token
{

public:

    Base_token()
    {

    }

     virtual ~Base_token() {}

};

class Text :  public Base_token {
public:
    Text(const QString& text) : text(text) {}

    QString text;
};

class Tag : public Base_token {
public:
    Tag(const QString& tag) : tag(tag) {}

    QString tag;
};

struct DisplayItem {
    int x;
    int y;
    QString word;
    QFont font;
};



class Layout {
public:
    Layout(const QList<Base_token *>& tokens) : tokens(tokens) {
        x = HSTEP;
        y = VSTEP;
        weight = "normal";
        style = "roman";
        size = 16;



        for(Base_token * toke : tokens){
            token(toke);
        }

        flush();
    }

    void token(Base_token * tok) {

        if (auto text = dynamic_cast<Text*>(tok)) {
            this->text(text->text);
            return;

        }

        auto tag_type = dynamic_cast<Tag*>(tok)->tag;

        qDebug() << tag_type;

        if (tag_type == "i") {
            style = "italic";
        }

        else if (tag_type == "/i") {
            style = "roman";
        } else if (tag_type == "b") {
            weight = "bold";
        } else if (tag_type == "/b") {
            weight = "normal";
        } else if (tag_type == "small") {
            size -= 2;
        } else if (tag_type == "/small") {
            size += 2;
        } else if (tag_type == "big") {
            size += 4;
        } else if (tag_type == "/big") {
            size -= 4;
        } else if (tag_type == "br") {
            flush();
        } else if (tag_type == "/p") {
            qDebug() << "testezdf";
           // y += VSTEP;
            flush();

        } else if (tag_type == "p")
        {
            qDebug() << "testezdf";
            y += VSTEP;
            flush();

        }



    }

    void text(const QString& text) {
        QFont font;
        font.setPointSize(size);
        font.setWeight(weight == "bold" ? QFont::Bold : QFont::Normal);
        font.setStyleName(style);
        QStringList words = text.split(' ');

        for (const QString& word : words) {
            int w =  QFontMetrics(font).horizontalAdvance(word);
            if (x + w > WIDTH - HSTEP) {
                flush();
            }
            line.append(DisplayItem{x, y, word, font});
            x += w + QFontMetrics(font).horizontalAdvance(' ');
        }
    }

    void flush() {
        if (line.isEmpty()) return;
        QList<QFontMetrics> metrics;
        for (const DisplayItem& item : line) {
            metrics.append(QFontMetrics(item.font));
        }
        int maxAscent = 0;
        for (const QFontMetrics& metric : metrics) {
            maxAscent = qMax(maxAscent, metric.ascent());
        }
        qreal baseline = y + 1.2 * maxAscent;
        for (const DisplayItem& item : line) {
            int x = item.x;
            const QString& word = item.word;
            const QFont& font = item.font;
            int y = baseline - QFontMetrics(font).ascent();
            displayList.append({x, y, word, font});
        }
        x = HSTEP;
        line.clear();
        int maxDescent = 0;
        for (const QFontMetrics& metric : metrics) {
            maxDescent = qMax(maxDescent, metric.descent());
        }
        y = baseline + 1.2 * maxDescent;
    }

    QList<DisplayItem> displayList;

private:
    QList<Base_token *> tokens;
    QList<DisplayItem> line;
    int x;
    int y;
    QString weight;
    QString style;
    int size;
};


QList<Base_token *> lex(const QString& body)
{
    QList<Base_token *> out;
    QString text = "";
    bool in_tag = false;

    for (const QChar& c : body) {
        if (c == '<') {
            in_tag = true;
            if (!text.isEmpty())
                out.append(new Text(text));
            text = "";
        } else if (c == '>') {
            in_tag = false;
            out.append(new Tag(text));
            text = "";
        } else {
            text += c;
        }
    }

    if (!in_tag && !text.isEmpty()) {
        out.append(new Text(text));
    }

    return out;
}
class Browser : public QGraphicsView {

public:
    Browser(QWidget* parent = nullptr) : QGraphicsView(parent), scroll(0)
    {
        setRenderHint(QPainter::Antialiasing);
        setRenderHint(QPainter::TextAntialiasing);
        setRenderHint(QPainter::SmoothPixmapTransform);
        //setRenderHint(QPainter::HighQualityAntialiasing);
        setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
        setScene(new QGraphicsScene(this));

        scene()->setBackgroundBrush(QBrush(Qt::white));

    }

    virtual ~Browser() {}

    void layout(QList<Base_token *> tokens)
    {
        displayList = Layout(tokens).displayList;
        render();
    }

    void render() {

        scene()->clear();

        qDebug() <<  displayList.size();
        for (const DisplayItem& item : displayList) {
            int x = item.x;
            int y = item.y - scroll;
            const QString& word = item.word;
            const QFont& font = item.font;

            qDebug() <<  word;
            if (y > HEIGHT)
                continue;
            if (y + font.letterSpacing() < -scroll)
                continue;
            QGraphicsTextItem* textItem = scene()->addText(word, font);
            textItem->setPos(x, y);
        }
    }

protected:
    void keyPressEvent(QKeyEvent* event) {
        if (event->key() == Qt::Key_Down) {
            scroll += SCROLL_STEP;
            render();
        }
    }

    int scroll;
    QList<DisplayItem> displayList;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    Browser browser;
    browser.setWindowTitle("Athenas Browser");
    browser.setGeometry(100, 100, WIDTH, HEIGHT);

    QList<Base_token *> tokens = lex("smafdfhj <p>teste();</p>"); // Replace this with your actual list of tokens.

    browser.layout(tokens);
    browser.show();
    return app.exec();
}
