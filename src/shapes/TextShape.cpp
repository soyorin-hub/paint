#include "TextShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFontMetricsF>

TextShape::TextShape(QGraphicsItem *parent)
    : ShapeBase(parent)
{
    m_text = QStringLiteral("文本");
    m_font = QFont("Microsoft YaHei", 18);
    m_finished = true;
    m_style.fillColor = Qt::transparent;
    m_style.strokeColor = Qt::black;  // 文字用描边色作为文字色
}

QRectF TextShape::boundingRect() const
{
    QFontMetricsF fm(m_font);
    QRectF br = fm.boundingRect(QRectF(0, 0, 1000, 1000),
                                 Qt::AlignLeft | Qt::AlignTop, m_text.isEmpty() ? " " : m_text);
    br.adjust(-6, -4 - HANDLE_TOP_CLEARANCE, 6, 6);
    return br;
}

QPainterPath TextShape::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void TextShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setFont(m_font);

    // 文字颜色用描边色（如果透明则用黑色）
    QColor textColor = m_style.strokeColor;
    if (textColor.alpha() == 0 || !textColor.isValid())
        textColor = Qt::black;
    painter->setPen(textColor);
    painter->setBrush(Qt::NoBrush);

    QRectF br = boundingRect();
    painter->drawText(br.adjusted(4, 2, -4, -2), Qt::AlignLeft | Qt::AlignTop, m_text);

    // 选中时显示虚线框 + 手柄
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(br);

        paintHandles(painter, br);
    }
}

void TextShape::setP2(const QPointF &p2)
{
    Q_UNUSED(p2)
    m_finished = true;
    update();
}

void TextShape::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        prepareGeometryChange();
        update();
    }
}

void TextShape::setFont(const QFont &font)
{
    m_font = font;
    prepareGeometryChange();
    update();
}

QJsonObject TextShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"]     = "TextShape";
    obj["text"]     = m_text;
    obj["fontFamily"] = m_font.family();
    obj["fontSize"]   = m_font.pointSize();
    obj["fontBold"]   = m_font.bold();
    obj["fontItalic"] = m_font.italic();
    return obj;
}

void TextShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    m_text = obj["text"].toString(QStringLiteral("文本"));
    QString family = obj["fontFamily"].toString("Microsoft YaHei");
    int size = obj["fontSize"].toInt(18);
    m_font = QFont(family, size);
    m_font.setBold(obj["fontBold"].toBool(false));
    m_font.setItalic(obj["fontItalic"].toBool(false));
    update();
}
