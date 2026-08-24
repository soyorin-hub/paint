#include "RectShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>
#include <QLineF>
#include <cmath>

namespace {
QPointF towardPoint(const QPointF &from, const QPointF &to, qreal d)
{
    qreal len = QLineF(from, to).length();
    if (len < 1e-9) return from;
    return from + (to - from) * (d / len);
}

// 圆角多边形路径（每角独立半径，逐角钳制避免相邻圆角重叠）
QPainterPath roundedPolygonPath(const QVector<QPointF> &pts, const QVector<qreal> &radii)
{
    QPainterPath path;
    int n = pts.size();
    if (n < 3) return path;

    QVector<qreal> r(n);
    for (int i = 0; i < n; ++i) {
        int p = (i + n - 1) % n;
        int q = (i + 1) % n;
        qreal maxR = qMin(QLineF(pts[p], pts[i]).length(),
                          QLineF(pts[i], pts[q]).length()) / 2.0;
        r[i] = qBound(0.0, radii[i], maxR);
    }

    path.moveTo(towardPoint(pts[0], pts[n - 1], r[0]));
    for (int i = 0; i < n; ++i) {
        int p = (i + n - 1) % n;
        int q = (i + 1) % n;
        path.lineTo(towardPoint(pts[i], pts[p], r[i]));
        path.quadTo(pts[i], towardPoint(pts[i], pts[q], r[i]));
    }
    path.closeSubpath();
    return path;
}
} // namespace

RectShape::RectShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
    setDefaultVertices(m_rect);
}

RectShape::RectShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
    setDefaultVertices(m_rect);
}

void RectShape::setDefaultVertices(const QRectF &r)
{
    m_vertices.clear();
    m_vertices << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();
}

void RectShape::updateRectFromVertices()
{
    m_rect = m_vertices.boundingRect();
}

QPointF RectShape::cornerBisector(int index) const
{
    int n = m_vertices.size();
    if (n < 3) return QPointF(0, 0);
    int p = (index + n - 1) % n;
    int q = (index + 1) % n;
    QPointF e1 = m_vertices[p] - m_vertices[index];
    QPointF e2 = m_vertices[q] - m_vertices[index];
    qreal l1 = QLineF(m_vertices[index], m_vertices[p]).length();
    qreal l2 = QLineF(m_vertices[index], m_vertices[q]).length();
    if (l1 < 1e-9 || l2 < 1e-9) return QPointF(0, 0);
    QPointF d = QPointF(e1.x() / l1, e1.y() / l1) + QPointF(e2.x() / l2, e2.y() / l2);
    qreal dl = std::hypot(d.x(), d.y());
    if (dl < 1e-9) return QPointF(0, 0);
    return QPointF(d.x() / dl, d.y() / dl);
}

QRectF RectShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath RectShape::shape() const
{
    return roundedPolygonPath(m_vertices, { m_radiusTL, m_radiusTR, m_radiusBR, m_radiusBL });
}

void RectShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                      QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());
    painter->drawPath(shape());

    // 选中态高亮
    if (option->state & QStyle::State_Selected) {
        if (m_directSelected) {
            paintDirectSelectionHighlights(painter);
        } else {
            painter->setPen(QPen(QColor(0, 120, 215), 1.0, Qt::DashLine));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(m_rect.adjusted(-3, -3, 3, 3));
            paintHandles(painter, m_rect);
        }
    }
}

void RectShape::setP2(const QPointF &p2)
{
    setRect(QRectF(m_rect.topLeft(), p2).normalized());
    m_finished = false;
}

void RectShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    setDefaultVertices(m_rect);
    update();
}

void RectShape::setCornerRadius(qreal tl, qreal tr, qreal br, qreal bl)
{
    qreal maxR = qMin(m_rect.width(), m_rect.height()) / 2.0;
    m_radiusTL = qBound(0.0, tl, maxR);
    m_radiusTR = qBound(0.0, tr, maxR);
    m_radiusBR = qBound(0.0, br, maxR);
    m_radiusBL = qBound(0.0, bl, maxR);
    update();
}

QVector<QPointF> RectShape::cornerRadiusHandlePositions() const
{
    const qreal inset = 7.0;
    qreal radii[4] = { m_radiusTL, m_radiusTR, m_radiusBR, m_radiusBL };
    QVector<QPointF> pts;
    for (int i = 0; i < 4; ++i)
        pts.append(m_vertices[i] + cornerBisector(i) * qMax(radii[i], inset));
    return pts;
}

void RectShape::setCornerRadiusFromPoint(int index, const QPointF &localPt)
{
    if (index < 0 || index >= 4) return;
    QPointF d = localPt - m_vertices[index];
    qreal r = QPointF::dotProduct(d, cornerBisector(index));
    switch (index) {
    case 0: setCornerRadius(r, m_radiusTR, m_radiusBR, m_radiusBL); break;
    case 1: setCornerRadius(m_radiusTL, r, m_radiusBR, m_radiusBL); break;
    case 2: setCornerRadius(m_radiusTL, m_radiusTR, r, m_radiusBL); break;
    case 3: setCornerRadius(m_radiusTL, m_radiusTR, m_radiusBR, r); break;
    }
}

QVector<QPointF> RectShape::anchorPoints() const
{
    return m_vertices;
}

void RectShape::setAnchorPoint(int index, const QPointF &pt)
{
    if (index < 0 || index >= m_vertices.size()) return;
    if (m_vertices[index] == pt) return;
    m_vertices[index] = pt;
    updateRectFromVertices();
    update();
}

void RectShape::setAnchorPoints(const QVector<QPointF> &points)
{
    if (points.size() != 4) return;
    m_vertices = points;
    updateRectFromVertices();
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
    if (m_radiusTL > 0) obj["cornerRadiusTL"] = m_radiusTL;
    if (m_radiusTR > 0) obj["cornerRadiusTR"] = m_radiusTR;
    if (m_radiusBR > 0) obj["cornerRadiusBR"] = m_radiusBR;
    if (m_radiusBL > 0) obj["cornerRadiusBL"] = m_radiusBL;
    // 四角相等时兼容旧格式
    if (m_radiusTL > 0 && m_radiusTL == m_radiusTR
        && m_radiusTR == m_radiusBR && m_radiusBR == m_radiusBL)
        obj["cornerRadius"] = m_radiusTL;

    QJsonArray verts;
    for (const QPointF &p : m_vertices) {
        QJsonObject o;
        o["x"] = p.x();
        o["y"] = p.y();
        verts.append(o);
    }
    obj["vertices"] = verts;
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

    QJsonArray verts = obj["vertices"].toArray();
    if (verts.size() == 4) {
        m_vertices.clear();
        for (int i = 0; i < 4; ++i) {
            QJsonObject o = verts[i].toObject();
            m_vertices << QPointF(o["x"].toDouble(), o["y"].toDouble());
        }
    } else {
        setDefaultVertices(m_rect);
    }

    qreal legacy = obj["cornerRadius"].toDouble(0);
    m_radiusTL = obj["cornerRadiusTL"].toDouble(legacy);
    m_radiusTR = obj["cornerRadiusTR"].toDouble(legacy);
    m_radiusBR = obj["cornerRadiusBR"].toDouble(legacy);
    m_radiusBL = obj["cornerRadiusBL"].toDouble(legacy);
    update();
}
