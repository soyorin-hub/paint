#include "ShapeStyle.h"
#include <QJsonArray>

QPen ShapeStyle::toPen() const
{
    QPen pen(strokeColor, strokeWidth);
    pen.setStyle(penStyle);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    return pen;
}

QBrush ShapeStyle::toBrush() const
{
    return QBrush(fillColor);
}

QJsonObject ShapeStyle::toJson() const
{
    QJsonObject obj;
    obj["fillColor"]   = fillColor.name(QColor::HexArgb);
    obj["strokeColor"] = strokeColor.name(QColor::HexArgb);
    obj["strokeWidth"] = strokeWidth;
    obj["penStyle"]    = static_cast<int>(penStyle);
    return obj;
}

void ShapeStyle::fromJson(const QJsonObject &obj)
{
    if (obj.contains("fillColor"))
        fillColor = QColor(obj["fillColor"].toString());
    if (obj.contains("strokeColor"))
        strokeColor = QColor(obj["strokeColor"].toString());
    if (obj.contains("strokeWidth"))
        strokeWidth = obj["strokeWidth"].toDouble(2.0);
    if (obj.contains("penStyle"))
        penStyle = static_cast<Qt::PenStyle>(obj["penStyle"].toInt(Qt::SolidLine));
}

bool ShapeStyle::operator==(const ShapeStyle &other) const
{
    return fillColor == other.fillColor
        && strokeColor == other.strokeColor
        && qFuzzyCompare(strokeWidth, other.strokeWidth)
        && penStyle == other.penStyle;
}
