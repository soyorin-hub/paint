#include "TriangleShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

TriangleShape::TriangleShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
}

TriangleShape::TriangleShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
}

QRectF TriangleShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath TriangleShape::shape() const
{
    QPainterPath path;
    QPolygonF triangle;
    triangle << QPointF(m_rect.left() + m_rect.width() / 2.0, m_rect.top())
             << QPointF(m_rect.right(), m_rect.bottom())
             << QPointF(m_rect.left(), m_rect.bottom());
    path.addPolygon(triangle);
    return path;
}

void TriangleShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());

    QPolygonF triangle;
    triangle << QPointF(m_rect.left() + m_rect.width() / 2.0, m_rect.top())
             << QPointF(m_rect.right(), m_rect.bottom())
             << QPointF(m_rect.left(), m_rect.bottom());
    painter->drawPolygon(triangle);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect.adjusted(-3, -3, 3, 3));

        paintHandles(painter, m_rect);
    }
}

void TriangleShape::setP2(const QPointF &p2)
{
    m_rect = QRectF(m_rect.topLeft(), p2).normalized();
    m_finished = false;
    update();
}

void TriangleShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    update();
}

QJsonObject TriangleShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "TriangleShape";
    obj["x"] = m_rect.x();
    obj["y"] = m_rect.y();
    obj["width"]  = m_rect.width();
    obj["height"] = m_rect.height();
    return obj;
}

void TriangleShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    qreal x = obj["x"].toDouble(0);
    qreal y = obj["y"].toDouble(0);
    qreal w = obj["width"].toDouble(100);
    qreal h = obj["height"].toDouble(80);
    m_rect = QRectF(x, y, w, h);
    update();
}
