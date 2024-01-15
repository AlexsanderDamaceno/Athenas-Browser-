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

const QStringList BLOCK_ELEMENTS = {

    "html", "body", "article", "section", "nav", "aside",
    "h1", "h2", "h3", "h4", "h5", "h6", "hgroup", "header",
    "footer", "address", "p", "hr", "pre", "blockquote",
    "ol", "ul", "menu", "li", "dl", "dt", "dd", "figure",
    "figcaption", "main", "div", "table", "form", "fieldset",
    "legend", "details", "summary"

};


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

const int WIDTH = 800;
const int HEIGHT = 600;
const int HSTEP = 13;
const int VSTEP = 18;
const int SCROLL_STEP = 100;

class Layout {
public:
    Layout(ElementNode* tree , Layout *parent , QVector<Layout *> children) :  parent(parent) , children(children) , x(HSTEP), y(VSTEP), weight("normal"), style("roman"), size(16) {

    }

    Layout()
    {

    }

    virtual void layout_node(){};


    virtual void layout()
    {


    }




    virtual QList<DrawCMD *> paint()
    {
            return QList<DrawCMD *>();
    }

    QList<DisplayItem*> displayList;

    int x = 0;
    int y = 0;
    int width  = 0;
    int height = 0;

    QString weight;
    QString style;
    int size = 16;
    QList<DisplayItem> line;
    QVector<Layout *> children;
    Layout *parent = nullptr;

};


int i = 0;

class InlineLayout : public Layout
{
public:
    InlineLayout(ElementNode* tree , Layout *parent , Layout * previus)
    {
            this->parent  = parent;
            this->previus = previus;
            this->x = HSTEP;
            this->y = VSTEP;
            this->weight = "normal";
            this->style = "roman";
            this->size = 16;
            this->node = tree;
            this->height = 0;
            this->width = 0;
            this->x = 0;
            this->y = 0;
            this->cursor_x = 0;
            this->cursor_y = 0;


    }

    InlineLayout()
    {

    }

    virtual void layout_node(){};


     void layout() override
    {
            width = parent->width;
            x = parent->x;

            if (this->previus) {
            qDebug() << "previ " << previus->height;
            y = previus->y + previus->height +  20;
            } else {
            y = parent->y;
            }

            displayList.clear();
            weight = "normal";
            style = "roman";
            size = 16;

            this->cursor_x = x;
            this->cursor_y = y;
            line.clear();
            recurse(node);
            flush();

            height = this->cursor_y - y;
    }

    void recurse(ElementNode* tree) {

            //qDebug() << "[RECURSE]";
            // open(tree->tag);
            if(dynamic_cast<TagNode*>(tree))
            open(static_cast<TagNode*>(tree)->tag);


            foreach (ElementNode* child, tree->children) {
            if (dynamic_cast<TextNode*>(child)) {

                text(static_cast<TextNode*>(child)->text);
            } else {
                recurse(child);
            }
            }

            if(dynamic_cast<TagNode*>(tree))
            close(static_cast<TagNode*>(tree)->tag);


            // close(tree->tag);
    }

    void open(const QString& tag) {
            if (tag == "i") {
            style = "italic";
            } else if (tag == "b") {
            weight = "bold";
            } else if (tag == "small") {
            size -= 2;
            } else if (tag == "big") {
            size += 4;
            } else if (tag == "br") {
            flush();
            }
    }

    void close(const QString& tag) {
            if (tag == "i") {
            style = "roman";
            } else if (tag == "b") {
            weight = "normal";
            } else if (tag == "small") {
            size += 2;
            } else if (tag == "big") {
            size -= 4;
            } else if (tag == "p") {
            flush();
            y += VSTEP;
            }
    }

    void text(const QString& text) {
            QFont font;
            font.setPixelSize(10);
            font.setWeight(QFont::Normal);
            font.setStyle(QFont::StyleNormal);
            font.setItalic(false);

            QStringList words = text.split(' ');
            for (const QString& word : words) {
            int w = QFontMetrics(font).horizontalAdvance(word);
            if (x + w > WIDTH - HSTEP) {
                flush();
            }
            line.append({x, y, word, font});

            x += w + QFontMetrics(font).horizontalAdvance(' ');
            }


    }

    void flush() {

            // qDebug() << "[FLUSH] ";
            if (line.isEmpty()) return;


            QFont font;
            font.setPixelSize(10);
            font.setWeight((weight == "bold") ? QFont::Bold : QFont::Normal);
            font.setStyle((style == "italic") ? QFont::StyleItalic : QFont::StyleNormal);

            int maxAscent = 0;
            for (const DisplayItem& item : line) {
            maxAscent = qMax(maxAscent, QFontMetrics(font).ascent());
            }
            qreal baseline = y + 1.2 * maxAscent;
            for (const DisplayItem& item : line) {
            int x = item.x;
            int y = baseline - QFontMetrics(font).ascent();
            //  qDebug() << "[FLUSH] 2";

            DisplayItem* tmp = new  DisplayItem(x, y, item.word, font);
            displayList.append(tmp);
            }
            x = HSTEP;
            line.clear();
            int maxDescent = 0;
            for ( DisplayItem* item : displayList) {
            maxDescent = qMax(maxDescent, QFontMetrics(item->font).descent());
            }
            y = baseline + 1.2 * maxDescent;


    }

     QList<DrawCMD *> paint() override
    {


            QList<DrawCMD *> cmds;
            for (auto* entry : displayList)
            {
               cmds.append(new DrawText(entry->x, entry->y, entry->word, entry->font));
            }

            return cmds;

    }

    Layout *previus = nullptr;
    int cursor_x = 0;
    int cursor_y = 0;
    ElementNode *node = nullptr;

};

class BlockLayout : public Layout
{
public:
    BlockLayout(ElementNode* node, Layout* parent, Layout* previous)
        : node(node) {

            this->parent = parent;
            this->height = 0;
            this->weight = 0;
            this->x = 0;
            this->y = 0;
            this->previous = previous;
    }

    void layoutIntermediate()

    {

        BlockLayout* previous = nullptr;
        for (auto * child : node->children)
        {
            BlockLayout* next = new BlockLayout(child, this, previous);
            children.append(next);
            previous = next;
        }

    }

     void layout() override
    {


        this->x      = this->parent->x;
        this->width  = this->parent->width;

        if(this->previous)
        {
            this->y = this->previous->y + this->previous->height;
        }else{
            this->y = this->parent->y;
        }

        QString mode = layoutMode();

        if (mode == "block") {


            Layout* previous = nullptr;
            for (auto * child : this->node->children) {
                if(layoutMode(child) == "inline" )
                {

                 InlineLayout* next = new InlineLayout(child, this, previous);
                 children.append(next);
                 previous = next;
                }else
                {

                BlockLayout* next = new BlockLayout(child, this, previous);
                children.append(next);
                previous = next;
                }
            }
        }

        for(auto *child : this->children)
        {

            child->layout();
        }

        if(mode  == "block")
        {
            for(auto *child : this->children)
            {

                this->height += child->height;

            }
        }


        for(auto *child : this->children)
        {
            for(auto *node: child->displayList)
                this->display_list.append(node);
        }

    }


    QString layoutMode() const
    {

        if (dynamic_cast<TextElement*>(node)) {
            return "inline";
        } else if (std::find_if(this->node->children.begin(), this->node->children.end(),
                                [](ElementNode* child)
                                {
                                       TagNode* tagElement = dynamic_cast<TagNode*>(child);



                                  return (tagElement && BLOCK_ELEMENTS.contains(tagElement->tag));
                   }) != node->children.end()) {
            return "block";
        } else if (node->children.size() > 0) {
            return "inline";
        } else {
            return "block";
        }
    }

    QString layoutMode(ElementNode *node) const
    {

        if (dynamic_cast<TextElement*>(node)) {
            return "inline";
        } else if (std::find_if(node->children.begin(), node->children.end(),
                                [](ElementNode* child)
                                {
                                    TagNode* tagElement = dynamic_cast<TagNode*>(child);

                       if(!tagElement)
                                    {
                                        return false;
                                    }



                                    return (tagElement && BLOCK_ELEMENTS.contains(tagElement->tag));
                                }) != node->children.end()) {
            return "block";
        } else if (node->children.size() > 0) {
            return "inline";
        } else {
            return "block";
        }
    }

    void word(QString word)
    {


        QFont font("Helvetica");  // getFont(size, weight, style);

        font.setPointSize(20);

        int w = QFontMetrics(font).horizontalAdvance(word);
        if (cursor_x + w > width) {
            flush();
        }
        line.append({cursor_x , cursor_y, word, font});
        cursor_x += w +  QFontMetrics(font).horizontalAdvance(word);


    }

    void flush() {

       // qDebug() << "[FLUSH] ";


        if (line.isEmpty()) return;
        QFont font;

        cursor_x = 0;
        cursor_y = 0;

        font.setPixelSize(10);
        font.setWeight((weight == "bold") ? QFont::Bold : QFont::Normal);
        font.setStyle((style == "italic") ? QFont::StyleItalic : QFont::StyleNormal);

        int maxAscent = 0;
        for (const DisplayItem& item : line) {
            maxAscent = qMax(maxAscent, QFontMetrics(font).ascent());
        }
        qreal baseline = y + 1.2 * maxAscent;
        for (const DisplayItem& item : line) {
            int x = this->x + item.x;
            int y = this->y + baseline - QFontMetrics(font).ascent();
            qDebug() << "[FLUSH] 2";

            auto *tmp_display_item = new DisplayItem(x , y , item.word , font);
            this->display_list.append(tmp_display_item);
        }
        x = HSTEP;

        line.clear();

        int maxDescent = 0;

        for (DisplayItem * item : this->display_list) {
            maxDescent = qMax(maxDescent, QFontMetrics(item->font).descent());
        }

        y = baseline + 1.2 * maxDescent;


    }



    QList<DrawCMD *> paint() override
    {

        QList<DrawCMD *> cmds;

        if (dynamic_cast<TagNode*>(node))
        {
            qDebug() << "value " << static_cast<TagNode *>(node)->tag ;
        }

        if (dynamic_cast<TagNode*>(node) && static_cast<TagNode *>(node)->tag == "html")
        {
            qDebug()<< "dafkljadsfds" << height;
            int x2 = x + width;
            int y2 = y + height;
            DrawRect* rect = new DrawRect(x, y, x2, y2, QColor("green"));
            cmds.append(rect);
        }

        if (layoutMode() == "inline")
        {

            for (const auto& display : displayList) {

                int x = display->x;
                int y = display->y;
                const QString& word = display->word;
                const QFont& font = display->font;

                DrawText* text = new DrawText(x, y, word, font);
                cmds.append(text);

            }
        }

        return cmds;

    }




    QList<DisplayItem *> display_list;

private:

    ElementNode* node;
   // Layout* parent;
    Layout* previous = nullptr;


    int cursor_x = 0;
    int cursor_y = 0;

    QString weight = "normal";
    QString style  = "roman";

    int size = 32;

    QList<DisplayItem> line;





};

class DocumentLayout : public Layout
{
public:
    DocumentLayout(ElementNode* node) : Element_node(node)
    {
        this->parent = nullptr;
    }


    QList<DrawCMD *>  paint() override
    {

        return  QList<DrawCMD *>();

    }


    void layout() override
    {



        BlockLayout* child = new BlockLayout(Element_node , this , nullptr);
        children.append(child);


        width = WIDTH - 2 *  HSTEP;
        x = HSTEP;
        y = VSTEP;


        child->layout();

        this->display_item =  child->display_list;
        this->height = child->height;


    }

  //  QList<DrawCMD *>  paint()
   // {
   //     return QList<DrawCMD *>();
   // }

    QList<DisplayItem *> display_item;
private:

    ElementNode* Element_node;



};


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
    // set icon




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


