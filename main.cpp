#include "mainwindow.h"

#include <QApplication>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QScrollBar>
#include <QKeyEvent>
#include <QFontMetrics>
#include <QDebug>
#include <algorithm>
#include "./includes/command/draw.hpp"
#include "./includes/elements//elements.hpp"
#include "./includes/html/html.hpp"
#include "includes/layout/layout.hpp"



class Browser : public QMainWindow {


public:
    Browser(QWidget* parent = nullptr) : QMainWindow(parent), scroll(0)
    {
        QGraphicsScene* scene = new QGraphicsScene(this);
        view = new QGraphicsView(scene, this);
        setCentralWidget(view);
        setFixedSize(WIDTH, HEIGHT);
        setWindowTitle("Athenas Browser");

        QScrollBar* scrollbar = new QScrollBar(Qt::Vertical, this);
        view->setVerticalScrollBar(scrollbar);
        connect(scrollbar, &QScrollBar::valueChanged, this, &Browser::scrollChanged);

        scene->setSceneRect(0, 0, WIDTH, HEIGHT);

        QColor backgroundColor = QColor(Qt::white);
        QBrush backgroundBrush(backgroundColor);
        scene->setBackgroundBrush(backgroundBrush);


    }


    void layout(const QString& body)
    {
        QList<Element*> tokens    = HTMLParser(body).parse();

        QList<ElementNode*> tree  = HTMLParser(body).parseTokens(tokens);

        Document_Layout = new DocumentLayout(tree.first());

        Document_Layout->layout();

        paintTree(Document_Layout);

        render();

    }

    void paintTree(Layout *object)
    {


        for(auto  *cmd : object->paint())
        {
             displayList.append(cmd);
        }


        for (auto * child : object->children) {
            paintTree(child);
        }
    }

    void render() {

        QGraphicsScene* scene = view->scene();


        scene->clear();

        QFont font;
        font.setStyle(QFont::StyleNormal);





        for(DrawCMD * command : this->displayList)
        {

            if(dynamic_cast<DrawText *>(command))
            {


                auto item = static_cast<DrawText *>(command);

                int x = item->left;
                int y = item->top;

                const QString& word = item->text;
                const QFont& font = item->font;

                if (y > scroll + HEIGHT) continue;
                if (y + QFontMetrics(font).lineSpacing() < scroll) continue;

                QGraphicsTextItem* textItem = new QGraphicsTextItem(word);
                textItem->setFont(font);
                textItem->setPos(x, y - scroll);
                scene->addItem(textItem);

            }else if(dynamic_cast<DrawRect *>(command))
            {

                auto item = static_cast<DrawRect *>(command);

                QGraphicsRectItem* rectItem = new QGraphicsRectItem(
                    item->left, item->top - scroll,
                    item->right - item->left, item->bottom - item->top
                    );

                rectItem->setPen(Qt::NoPen);
                rectItem->setBrush(item->color);

                scene->addItem(rectItem);
            }

        }




/*
        for (const DisplayItem& item : displayList) {

            qDebug() << "teste ";
            int x = item.x;
            int y = item.y;
            const QString& word = item.word;
            const QFont& font = item.font;

            if (y > scroll + HEIGHT) continue;
            if (y + QFontMetrics(font).lineSpacing() < scroll) continue;

            QGraphicsTextItem* textItem = new QGraphicsTextItem(word);
            textItem->setFont(font);
            textItem->setPos(x, y - scroll);
            scene->addItem(textItem);
        }

*/
    }


    void executeDisplayList() {
      //  for (auto* cmd : displayList) {
     //       if (cmd->top > scroll + HEIGHT) continue;
     //       if (cmd->bottom < scroll) continue;
     //       cmd->execute(scroll, canvas);
     //   }
    }

    void scrollChanged(int value) {
        scroll = value;
        render();
    }

private:
    QGraphicsView* view;
    int scroll;
    QList<DrawCMD *> displayList;
    DocumentLayout *Document_Layout = nullptr;
};



int main(int argc, char* argv[])
{


    QApplication app(argc, argv);

    QIcon appIcon("/home/alexsander/Downloads/t.jpg");

    QString body =  "<html><body> <h1>Welcome to Athenas Browser! This browser is a peek into the magic of browser internals  in C++  </h1> <p><big>Paragraph 1</big></p> <p><big>Paragraph  2</big></p>  <p><big>Paragraph  3</big></p> "
                   "<b>Paragraph 4</b>  "
                   "  </body> </html> ";

    Browser browser;

    app.setWindowIcon(appIcon);

    browser.layout(body);
    browser.show();
    return app.exec();
}


