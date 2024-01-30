#pragma once

#include <QString>

class CSSParser
{
public:
    CSSParser(const QString& s);
    void whitespace();
    QString word();
    void literal(const QString& literal);

private:
    QString s;
    int i;
};

CSSParser::CSSParser(const QString& s) : s(s), i(0)
{
}

void CSSParser::whitespace()
{
    while (i < s.length() && s[i].isSpace())
    {
        i++;
    }
}

QString CSSParser::word()
{
    int start = i;
    while (i < s.length())
    {
        if (s[i].isLetterOrNumber() || s[i].in("#-.%"))
        {
            i++;
        }
        else
        {
            break;
        }
    }

    if (!(i > start))
    {
        throw std::runtime_error("Parsing error");
    }

    return s.mid(start, i - start);
}

void CSSParser::literal(const QString& literal)
{
    if (!(i < s.length() && s[i] == literal))
    {
        throw std::runtime_error("Parsing error");
    }

    i++;
}
