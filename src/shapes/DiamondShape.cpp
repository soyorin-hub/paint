#include "DiamondShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

DiamondShape::DiamondShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
}

DiamondShape::DiamondShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
}

QRectF DiamondShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath DiamondShape::shape() const
{
    QPainterPath path;
    QPolygonF diamond;
    diamond << QPointF(m_rect.center().x(), m_rect.top())       // 上
            << QPointF(m_rect.right(), m_rect.center().y())     // 右
            << QPointF(m_rect.center().x(), m_rect.bottom())    // 下
            << QPointF(m_rect.left(), m_rect.center().y());     // 左
    path.addPolygon(diamond);
    return path;
}

void DiamondShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());

    QPolygonF diamond;
    diamond << QPointF(m_rect.center().x(), m_rect.top())
            << QPointF(m_rect.right(), m_rect.center().y())
            << QPointF(m_rect.center().x(), m_rect.bottom())
            << QPointF(m_rect.left(), m_rect.center().y());
    painter->drawPolygon(diamond);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect.adjusted(-3, -3, 3, 3));

        paintHandles(painter, m_rect);
    }
}

void DiamondShape::setP2(const QPointF &p2)
{
    m_rect = QRectF(m_rect.topLeft(), p2).normalized();
    m_finished = false;
    update();
}

void DiamondShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    update();
}

QJsonObject DiamondShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "DiamondShape";
    obj["x"] = m_rect.x();
    obj["y"] = m_rect.y();
    obj["width"]  = m_rect.width();
    obj["height"] = m_rect.height();
    return obj;
}

void DiamondShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    qreal x = obj["x"].toDouble(0);
    qreal y = obj["y"].toDouble(0);
    qreal w = obj["width"].toDouble(100);
    qreal h = obj["height"].toDouble(80);
    m_rect = QRectF(x, y, w, h);
    update();
}
