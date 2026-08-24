#include "FreehandShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineF>

namespace {
// 点到线段 ab 的垂直距离（用于 RDP 简化）
qreal perpendicularDistance(const QPointF &p, const QPointF &a, const QPointF &b)
{
    QPointF ab = b - a;
    qreal len2 = ab.x() * ab.x() + ab.y() * ab.y();
    if (len2 < 1e-9)
        return QLineF(p, a).length();
    qreal t = qBound(0.0, QPointF::dotProduct(p - a, ab) / len2, 1.0);
    QPointF proj = a + t * ab;
    return QLineF(p, proj).length();
}

// Ramer-Douglas-Peucker：递归输出保留的顶点（out 需预先放入首点）
void rdpSimplify(const QVector<QPointF> &pts, qreal eps,
                 QVector<QPointF> &out, int first, int last)
{
    qreal maxDist = 0;
    int index = -1;
    for (int i = first + 1; i < last; ++i) {
        qreal d = perpendicularDistance(pts[i], pts[first], pts[last]);
        if (d > maxDist) { maxDist = d; index = i; }
    }
    if (maxDist > eps && index != -1) {
        rdpSimplify(pts, eps, out, first, index);
        rdpSimplify(pts, eps, out, index, last);
    } else {
        out.append(pts[last]);
    }
}
} // namespace

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
        if (m_directSelected) {
            paintDirectSelectionHighlights(painter);
        } else {
            QRectF br = m_cachedBoundingRect;
            painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(br.adjusted(-3, -3, 3, 3));

            paintHandles(painter, br);
        }
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

QVector<QPointF> FreehandShape::anchorPoints() const
{
    return m_points;
}

void FreehandShape::setAnchorPoint(int index, const QPointF &pt)
{
    if (index < 0 || index >= m_points.size()) return;
    if (m_points[index] == pt) return;
    m_points[index] = pt;
    rebuildPath();
    update();
}

void FreehandShape::setPoints(const QVector<QPointF> &points)
{
    m_points = points;
    rebuildPath();
    update();
}

void FreehandShape::setAnchorPoints(const QVector<QPointF> &points)
{
    setPoints(points);
}

QPainterPath FreehandShape::outlinePath() const
{
    return m_path;
}

void FreehandShape::simplify(qreal epsilon)
{
    if (m_points.size() <= 2) return;
    QVector<QPointF> out;
    out.reserve(m_points.size());
    out.append(m_points.first());
    rdpSimplify(m_points, epsilon, out, 0, m_points.size() - 1);
    m_points = out;
    rebuildPath();
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
