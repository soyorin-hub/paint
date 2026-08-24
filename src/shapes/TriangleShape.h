#ifndef TRIANGLESHAPE_H
#define TRIANGLESHAPE_H

#include "ShapeBase.h"
#include <QRectF>
#include <QPolygonF>

class TriangleShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 7 };
    int type() const override { return Type; }

    explicit TriangleShape(QGraphicsItem *parent = nullptr);
    TriangleShape(const QRectF &rect, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setP2(const QPointF &p2) override;
    bool isFinished() const override { return m_finished; }
    void setFinished(bool f) { m_finished = f; }

    QRectF rect() const { return m_rect; }
    void setRect(const QRectF &rect);
    QSizeF size() const override { return m_rect.size(); }
    void setSize(const QSizeF &size) override { setRect(QRectF(QPointF(0, 0), size)); }
    QRectF contentRect() const override { return m_rect; }

    // 顶点编辑
    QVector<QPointF> anchorPoints() const override;
    void setAnchorPoint(int index, const QPointF &pt) override;
    void setAnchorPoints(const QVector<QPointF> &points) override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QRectF m_rect;          // 包围盒（size/resize/对齐用）
    QPolygonF m_vertices;   // 3 顶点（本地坐标）
    void setDefaultVertices(const QRectF &r);
    void updateRectFromVertices();
};

#endif // TRIANGLESHAPE_H
