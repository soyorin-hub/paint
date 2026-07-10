#ifndef SHAPESTYLE_H
#define SHAPESTYLE_H

#include <QColor>
#include <QJsonObject>
#include <QPen>

struct ShapeStyle
{
    QColor fillColor   = Qt::white;
    QColor strokeColor = Qt::black;
    qreal  strokeWidth = 2.0;
    Qt::PenStyle penStyle = Qt::SolidLine;

    // 创建 QPen
    QPen toPen() const;

    // 创建 QBrush
    QBrush toBrush() const;

    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &obj);

    bool operator==(const ShapeStyle &other) const;
    bool operator!=(const ShapeStyle &other) const { return !(*this == other); }
};

#endif // SHAPESTYLE_H
