#include "EllipseShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>

namespace {
// 圆/椭圆 → 4 段三次贝塞尔的标准系数（0.5522847498）
constexpr qreal kBezierK = 0.552284749831;

// 4 个平滑锚点的固定切线方向（指向下一锚点，顺时针：上→右→下→左）
const QPointF kTangent[4] = {
    QPointF(1.0, 0.0), QPointF(0.0, 1.0), QPointF(-1.0, 0.0), QPointF(0.0, -1.0)
};
} // namespace

EllipseShape::EllipseShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
    setDefaultAnchors(m_rect);
}

EllipseShape::EllipseShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
    setDefaultAnchors(m_rect);
}

void EllipseShape::setDefaultAnchors(const QRectF &r)
{
    m_anchors.clear();
    m_anchors << QPointF(r.center().x(), r.top())    // 12 点
              << QPointF(r.right(), r.center().y())  // 3 点
              << QPointF(r.center().x(), r.bottom()) // 6 点
              << QPointF(r.left(), r.center().y());  // 9 点
}

void EllipseShape::updateRectFromAnchors()
{
    m_rect = m_anchors.boundingRect();
}

// 用 4 个平滑锚点构建闭合三次贝塞尔路径：
// 每段手柄沿固定切线方向，长度正比于弦在切线上的投影（保证曲线连续、初始为完美椭圆）
QPainterPath EllipseShape::buildPath() const
{
    QPainterPath path;
    if (m_anchors.size() != 4) return path;

    path.moveTo(m_anchors[0]);
    for (int i = 0; i < 4; ++i) {
        int j = (i + 1) % 4;
        QPointF chord = m_anchors[j] - m_anchors[i];
        qreal outLen = kBezierK * qMax(0.0, QPointF::dotProduct(chord, kTangent[i]));
        qreal inLen  = kBezierK * qMax(0.0, QPointF::dotProduct(chord, kTangent[j]));
        QPointF c1 = m_anchors[i] + kTangent[i] * outLen;
        QPointF c2 = m_anchors[j] - kTangent[j] * inLen;
        path.cubicTo(c1, c2, m_anchors[j]);
    }
    path.closeSubpath();
    return path;
}

QRectF EllipseShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath EllipseShape::shape() const
{
    return buildPath();
}

void EllipseShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                         QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());
    painter->drawPath(buildPath());

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

void EllipseShape::setP2(const QPointF &p2)
{
    setRect(QRectF(m_rect.topLeft(), p2).normalized());
    m_finished = false;
}

void EllipseShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    setDefaultAnchors(m_rect);
    update();
}

QVector<QPointF> EllipseShape::anchorPoints() const
{
    return m_anchors;
}

void EllipseShape::setAnchorPoint(int index, const QPointF &pt)
{
    if (index < 0 || index >= m_anchors.size()) return;
    if (m_anchors[index] == pt) return;
    m_anchors[index] = pt;
    updateRectFromAnchors();
    update();
}

void EllipseShape::setAnchorPoints(const QVector<QPointF> &points)
{
    if (points.size() != 4) return;
    m_anchors = points;
    updateRectFromAnchors();
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

    QJsonArray verts;
    for (const QPointF &p : m_anchors) {
        QJsonObject o;
        o["x"] = p.x();
        o["y"] = p.y();
        verts.append(o);
    }
    obj["vertices"] = verts;
    return obj;
}

void EllipseShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    qreal x = obj["x"].toDouble(0);
    qreal y = obj["y"].toDouble(0);
    qreal w = obj["width"].toDouble(100);
    qreal h = obj["height"].toDouble(80);
    m_rect = QRectF(x, y, w, h);

    QJsonArray verts = obj["vertices"].toArray();
    if (verts.size() == 4) {
        m_anchors.clear();
        for (int i = 0; i < 4; ++i) {
            QJsonObject o = verts[i].toObject();
            m_anchors << QPointF(o["x"].toDouble(), o["y"].toDouble());
        }
    } else {
        setDefaultAnchors(m_rect);
    }
    update();
}
