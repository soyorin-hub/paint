#include "LineShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPainterPath>

LineShape::LineShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(0, 0, 80, 0)
{
    m_finished = true;
    m_center = m_line.center();
}

LineShape::LineShape(const QLineF &line, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_line(line)
{
    m_finished = true;
    m_center = m_line.center();
}

QRectF LineShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 4.0;
    qreal x1 = qMin(m_line.x1(), qMin(m_center.x(), m_line.x2()));
    qreal y1 = qMin(m_line.y1(), qMin(m_center.y(), m_line.y2()));
    qreal x2 = qMax(m_line.x1(), qMax(m_center.x(), m_line.x2()));
    qreal y2 = qMax(m_line.y1(), qMax(m_center.y(), m_line.y2()));
    return QRectF(QPointF(x1, y1), QPointF(x2, y2))
        .adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath LineShape::shape() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_center);
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
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_center);
    path.lineTo(m_line.p2());
    painter->drawPath(path);

    if (option->state & QStyle::State_Selected) {
        if (m_directSelected)
            paintDirectSelectionHighlights(painter);
        else
            paintHandles(painter, QRectF());
    }
}

QVector<QPointF> LineShape::handlePositions() const
{
    return { m_line.p1(), m_center, m_line.p2() };
}

QPointF LineShape::linePoint(int index) const
{
    switch (index) {
    case 0:  return m_line.p1();
    case 1:  return m_center;
    default: return m_line.p2();
    }
}

void LineShape::setLinePoint(int index, const QPointF &pt)
{
    switch (index) {
    case 0:  m_line.setP1(pt); break;
    case 1:  m_center = pt;    break;
    default: m_line.setP2(pt); break;
    }
    update();
}

QVector<QPointF> LineShape::anchorPoints() const
{
    return { m_line.p1(), m_center, m_line.p2() };
}

void LineShape::setAnchorPoint(int index, const QPointF &pt)
{
    setLinePoint(index, pt);
}

void LineShape::setAnchorPoints(const QVector<QPointF> &points)
{
    if (points.size() < 3) return;
    m_line = QLineF(points[0], points[2]);
    m_center = points[1];
    update();
}

QPainterPath LineShape::outlinePath() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_center);
    path.lineTo(m_line.p2());
    return path;
}

void LineShape::setP2(const QPointF &p2)
{
    m_line.setP2(p2);
    m_center = m_line.center();
    m_finished = false;
    update();
}

void LineShape::setLine(const QLineF &line)
{
    m_line = line;
    m_center = line.center();
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
    obj["cx"] = m_center.x();
    obj["cy"] = m_center.y();
    return obj;
}

void LineShape::fromJson(const QJsonObject &obj)
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
