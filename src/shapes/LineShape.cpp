#include "LineShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <cmath>

LineShape::LineShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(0, 0, 80, 0)
{
    m_finished = true;
}

LineShape::LineShape(const QLineF &line, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(line)
{
    m_finished = true;
}

QRectF LineShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 4.0;
    return QRectF(m_line.p1(), m_line.p2()).normalized().adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath LineShape::shape() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());

    // 将 path 扩展成有一定宽度的区域用于碰撞检测
    QPainterPathStroker stroker;
    stroker.setWidth(qMax(m_style.strokeWidth, 6.0));
    return stroker.createStroke(path);
}

void LineShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->drawLine(m_line);

    if (option->state & QStyle::State_Selected) {
        QRectF br = boundingRect();
        qreal pad = m_style.strokeWidth / 2.0 + 2.0;
        QRectF selRect = br.adjusted(pad, pad, -pad, -pad);
        if (selRect.width() < 10) selRect.setWidth(10);
        if (selRect.height() < 10) selRect.setHeight(10);

        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(selRect.adjusted(-3, -3, 3, 3));

        paintHandles(painter, selRect);
    }
}

void LineShape::setP2(const QPointF &p2)
{
    m_line.setP2(p2);
    m_finished = false;
    update();
}

void LineShape::setLine(const QLineF &line)
{
    m_line = line;
    update();
}

QJsonObject LineShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "LineShape";
    obj["x1"] = m_line.x1();
    obj["y1"] = m_line.y1();
    obj["x2"] = m_line.x2();
    obj["y2"] = m_line.y2();
    return obj;
}

void LineShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    m_line = QLineF(obj["x1"].toDouble(0), obj["y1"].toDouble(0),
                    obj["x2"].toDouble(80), obj["y2"].toDouble(0));
    update();
}
