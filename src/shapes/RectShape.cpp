#include "RectShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

RectShape::RectShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
}

RectShape::RectShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
}

QRectF RectShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath RectShape::shape() const
{
    QPainterPath path;
    if (m_cornerRadius > 0) {
        path.addRoundedRect(m_rect, m_cornerRadius, m_cornerRadius);
    } else {
        path.addRect(m_rect);
    }
    return path;
}

void RectShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());

    if (m_cornerRadius > 0) {
        painter->drawRoundedRect(m_rect, m_cornerRadius, m_cornerRadius);
    } else {
        painter->drawRect(m_rect);
    }

    // 选中态高亮
    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect.adjusted(-3, -3, 3, 3));

        paintHandles(painter, m_rect);
    }
}

void RectShape::setP2(const QPointF &p2)
{
    m_rect = QRectF(m_rect.topLeft(), p2).normalized();
    m_finished = false;
    update();
}

void RectShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    update();
}

QJsonObject RectShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "RectShape";
    obj["x"] = m_rect.x();
    obj["y"] = m_rect.y();
    obj["width"]  = m_rect.width();
    obj["height"] = m_rect.height();
    if (m_cornerRadius > 0)
        obj["cornerRadius"] = m_cornerRadius;
    return obj;
}

void RectShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    qreal x = obj["x"].toDouble(0);
    qreal y = obj["y"].toDouble(0);
    qreal w = obj["width"].toDouble(100);
    qreal h = obj["height"].toDouble(80);
    m_rect = QRectF(x, y, w, h);
    m_cornerRadius = obj["cornerRadius"].toDouble(0);
    update();
}
