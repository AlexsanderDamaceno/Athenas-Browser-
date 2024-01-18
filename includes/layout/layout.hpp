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
#include "../command/draw.hpp"



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

        if (dynamic_cast<TagNode*>(node) && static_cast<TagNode *>(node)->tag == "body")
        {
            qDebug()<< "dafkljadsfds" << height;
            int x2 = x + width;
            int y2 = y + height;
            DrawRect* rect = new DrawRect(x, y, x2, y2, QColor("blue"));
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
