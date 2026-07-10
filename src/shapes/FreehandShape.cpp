#include "FreehandShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>

FreehandShape::FreehandShape(QGraphicsItem *parent)
    : ShapeBase(parent)
{
    m_finished = true;
    m_style.fillColor = Qt::transparent;
}

QRectF FreehandShape::boundingRect() const
{
    if (m_points.isEmpty())
        return QRectF();
    qreal pad = m_style.strokeWidth / 2.0 + 4.0;
    return m_cachedBoundingRect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath FreehandShape::shape() const
{
    QPainterPathStroker stroker;
    stroker.setWidth(qMax(m_style.strokeWidth, 6.0));
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    return stroker.createStroke(m_path);
}

void FreehandShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_path);

    if (option->state & QStyle::State_Selected) {
        QRectF br = m_cachedBoundingRect;
        painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(br.adjusted(-3, -3, 3, 3));

        paintHandles(painter, br);
    }
}

void FreehandShape::addPoint(const QPointF &point)
{
    m_points.append(point);

    // 重建路径并触发重绘（不调用 prepareGeometryChange，避免 BSP 树损坏）
    rebuildPath();
    update();
}

void FreehandShape::rebuildPath()
{
    if (m_points.isEmpty())
        return;

    m_path = QPainterPath();
    m_path.moveTo(m_points.first());
    for (int i = 1; i < m_points.size(); ++i) {
        m_path.lineTo(m_points[i]);
    }
    m_cachedBoundingRect = m_path.boundingRect();
    if (!m_points.isEmpty())
        m_lastPoint = m_points.last();
}

void FreehandShape::clearPath()
{
    m_points.clear();
    m_path = QPainterPath();
    m_cachedBoundingRect = QRectF();
    update();
}

void FreehandShape::setPath(const QPainterPath &path)
{
    m_path = path;
    m_points.clear();
    for (int i = 0; i < path.elementCount(); ++i) {
        QPainterPath::Element el = path.elementAt(i);
        m_points.append(QPointF(el.x, el.y));
    }
    m_cachedBoundingRect = m_path.boundingRect();
    update();
}

QJsonObject FreehandShape::toJson() const
{
    QJsonObject obj = ShapeBase::toJson();
    obj["type"] = "FreehandShape";

    QJsonArray points;
    for (int i = 0; i < m_points.size(); ++i) {
        QPointF pt = m_points[i];
        QJsonObject ptObj;
        ptObj["x"] = pt.x();
        ptObj["y"] = pt.y();
        // 第一个点标记为 MoveTo，其余为 LineTo
        ptObj["t"] = (i == 0) ? static_cast<int>(QPainterPath::MoveToElement)
                              : static_cast<int>(QPainterPath::LineToElement);
        points.append(ptObj);
    }
    obj["pathElements"] = points;
    return obj;
}

void FreehandShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    m_points.clear();

    QJsonArray points = obj["pathElements"].toArray();
    for (int i = 0; i < points.size(); ++i) {
        QJsonObject pt = points[i].toObject();
        qreal x = pt["x"].toDouble();
        qreal y = pt["y"].toDouble();
        m_points.append(QPointF(x, y));
    }
    rebuildPath();
    update();
}
