#include "TriangleShape.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>

TriangleShape::TriangleShape(QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(0, 0, 100, 80)
{
    m_finished = true;
    setDefaultVertices(m_rect);
}

TriangleShape::TriangleShape(const QRectF &rect, QGraphicsItem *parent)
    : ShapeBase(parent)
    , m_rect(rect.normalized())
{
    m_finished = true;
    setDefaultVertices(m_rect);
}

void TriangleShape::setDefaultVertices(const QRectF &r)
{
    m_vertices.clear();
    m_vertices << QPointF(r.center().x(), r.top())   // 顶点
               << QPointF(r.right(), r.bottom())
               << QPointF(r.left(), r.bottom());
}

void TriangleShape::updateRectFromVertices()
{
    m_rect = m_vertices.boundingRect();
}

QRectF TriangleShape::boundingRect() const
{
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    return m_rect.adjusted(-pad, -pad - HANDLE_TOP_CLEARANCE, pad, pad);
}

QPainterPath TriangleShape::shape() const
{
    QPainterPath path;
    path.addPolygon(m_vertices);
    return path;
}

void TriangleShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget)
{
    Q_UNUSED(widget)

    painter->setPen(m_style.toPen());
    painter->setBrush(m_style.toBrush());
    painter->drawPolygon(m_vertices);

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

void TriangleShape::setP2(const QPointF &p2)
{
    setRect(QRectF(m_rect.topLeft(), p2).normalized());
    m_finished = false;
}

void TriangleShape::setRect(const QRectF &rect)
{
    m_rect = rect.normalized();
    setDefaultVertices(m_rect);
    update();
}

QVector<QPointF> TriangleShape::anchorPoints() const
{
    return m_vertices;
}

void TriangleShape::setAnchorPoint(int index, const QPointF &pt)
{
    if (index < 0 || index >= m_vertices.size()) return;
    if (m_vertices[index] == pt) return;
    m_vertices[index] = pt;
    updateRectFromVertices();
    update();
}

void TriangleShape::setAnchorPoints(const QVector<QPointF> &points)
{
    if (points.size() != 3) return;
    m_vertices = points;
    updateRectFromVertices();
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

void TriangleShape::fromJson(const QJsonObject &obj)
{
    ShapeBase::fromJson(obj);
    qreal x = obj["x"].toDouble(0);
    qreal y = obj["y"].toDouble(0);
    qreal w = obj["width"].toDouble(100);
    qreal h = obj["height"].toDouble(80);
    m_rect = QRectF(x, y, w, h);

    QJsonArray verts = obj["vertices"].toArray();
    if (verts.size() == 3) {
        m_vertices.clear();
        for (int i = 0; i < 3; ++i) {
            QJsonObject o = verts[i].toObject();
            m_vertices << QPointF(o["x"].toDouble(), o["y"].toDouble());
        }
    } else {
        setDefaultVertices(m_rect);
    }
    update();
}
