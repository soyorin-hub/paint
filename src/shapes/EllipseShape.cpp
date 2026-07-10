#include "EllipseShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

EllipseShape::EllipseShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
}

EllipseShape::EllipseShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
}

QRectF EllipseShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath EllipseShape::shape() const
{
    QPainterPath path;
    path.addEllipse(m_rect);
    return path;
}

void EllipseShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());
    painter->drawEllipse(m_rect);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(m_rect.adjusted(-3, -3, 3, 3));

        paintHandles(painter, m_rect);
    }
}

void EllipseShape::setP2(const QPointF &p2)
{
    m_rect = QRectF(m_rect.topLeft(), p2).normalized();
    m_finished = false;
    update();
}

void EllipseShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    update();
}

QJsonObject EllipseShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "EllipseShape";
    obj["x"] = m_rect.x();
    obj["y"] = m_rect.y();
    obj["width"]  = m_rect.width();
    obj["height"] = m_rect.height();
    return obj;
}

void EllipseShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    m_rect = QRectF(obj["x"].toDouble(0), obj["y"].toDouble(0),
                    obj["width"].toDouble(100), obj["height"].toDouble(80));
    update();
}
