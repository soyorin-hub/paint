#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H

#include "ShapeBase.h"
#include <QRectF>
#include <QPolygonF>

class EllipseShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 3 };
    int type() const override { return Type; }

    explicit EllipseShape(QGraphicsItem *parent = nullptr);
    EllipseShape(const QRectF &rect, QGraphicsItem *parent = nullptr);

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

    // 顶点编辑（4 个平滑贝塞尔锚点：12/3/6/9 点钟）
    QVector<QPointF> anchorPoints() const override;
    void setAnchorPoint(int index, const QPointF &pt) override;
    void setAnchorPoints(const QVector<QPointF> &points) override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QPolygonF m_anchors;   // 4 锚点（12/3/6/9 点钟）
    QRectF m_rect;         // 包围盒（缓存）
    QPainterPath buildPath() const;
    void setDefaultAnchors(const QRectF &r);
    void updateRectFromAnchors();
};

#endif // ELLIPSESHAPE_H
