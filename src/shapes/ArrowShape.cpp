#include "ArrowShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPainterPath>
#include <cmath>

ArrowShape::ArrowShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(0, 0, 80, 0)
{
    m_finished = true;
    m_center = m_line.center();
}

ArrowShape::ArrowShape(const QLineF &line, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(line)
{
    m_finished = true;
    m_center = m_line.center();
}

QRectF ArrowShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 20.0; // extra for arrowhead
    qreal x1 = qMin(m_line.x1(), qMin(m_center.x(), m_line.x2()));
    qreal y1 = qMin(m_line.y1(), qMin(m_center.y(), m_line.y2()));
    qreal x2 = qMax(m_line.x1(), qMax(m_center.x(), m_line.x2()));
    qreal y2 = qMax(m_line.y1(), qMax(m_center.y(), m_line.y2()));
    return QRectF(QPointF(x1, y1), QPointF(x2, y2))
        .adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath ArrowShape::shape() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_center);
    path.lineTo(m_line.p2());

    // Arrowhead fins（方向沿最后一段 center -> p2）
    qreal arrowSize = 15.0;
    qreal angle = -QLineF(m_center, m_line.p2()).angle() * M_PI / 180.0;
    QPointF p2 = m_line.p2();
    qreal rad1 = angle + M_PI - M_PI / 6.0;
    qreal rad2 = angle + M_PI + M_PI / 6.0;
    path.moveTo(p2);
    path.lineTo(p2 + QPointF(std::cos(rad1), std::sin(rad1)) * arrowSize);
    path.moveTo(p2);
    path.lineTo(p2 + QPointF(std::cos(rad2), std::sin(rad2)) * arrowSize);

    QPainterPathStroker stroker;
    stroker.setWidth(qMax(m_style.strokeWidth, 6.0));
    return stroker.createStroke(path);
}

void ArrowShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());

    // 主体折线：起点 -> 中心 -> 终点
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_center);
    path.lineTo(m_line.p2());
    painter->drawPath(path);

    // 箭头（方向沿最后一段 center -> p2）
    qreal arrowSize = 15.0;
    qreal angle = -QLineF(m_center, m_line.p2()).angle() * M_PI / 180.0;
    QPointF p2 = m_line.p2();
    qreal rad1 = angle + M_PI - M_PI / 6.0;
    qreal rad2 = angle + M_PI + M_PI / 6.0;
    QPointF fin1 = p2 + QPointF(std::cos(rad1), std::sin(rad1)) * arrowSize;
    QPointF fin2 = p2 + QPointF(std::cos(rad2), std::sin(rad2)) * arrowSize;
    painter->drawLine(p2, fin1);
    painter->drawLine(p2, fin2);

    if (option->state & QStyle::State_Selected) {
        paintHandles(painter, QRectF());
    }
}

QVector<QPointF> ArrowShape::handlePositions() const
{
    return { m_line.p1(), m_center, m_line.p2() };
}

QPointF ArrowShape::linePoint(int index) const
{
    switch (index) {
    case 0:  return m_line.p1();
    case 1:  return m_center;
    default: return m_line.p2();
    }
}

void ArrowShape::setLinePoint(int index, const QPointF &pt)
{
    switch (index) {
    case 0:  m_line.setP1(pt); break;
    case 1:  m_center = pt;    break;
    default: m_line.setP2(pt); break;
    }
    update();
}

void ArrowShape::setP2(const QPointF &p2)
{
    m_line.setP2(p2);
    m_center = m_line.center();
    m_finished = false;
    update();
}

void ArrowShape::setLine(const QLineF &line)
{
    m_line = line;
    m_center = line.center();
    update();
}

QJsonObject ArrowShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "ArrowShape";
    obj["x1"] = m_line.x1();
    obj["y1"] = m_line.y1();
    obj["x2"] = m_line.x2();
    obj["y2"] = m_line.y2();
    obj["cx"] = m_center.x();
    obj["cy"] = m_center.y();
    return obj;
}

void ArrowShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    m_line = QLineF(obj["x1"].toDouble(0), obj["y1"].toDouble(0),
                    obj["x2"].toDouble(80), obj["y2"].toDouble(0));
    if (obj.contains("cx") && obj.contains("cy"))
        m_center = QPointF(obj["cx"].toDouble(), obj["cy"].toDouble());
    else
        m_center = m_line.center();
    update();
}
