#ifndef RECTSHAPE_H
#define RECTSHAPE_H

#include "ShapeBase.h"
#include <QRectF>
#include <QPolygonF>

class RectShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    explicit RectShape(QGraphicsItem *parent = nullptr);
    RectShape(const QRectF &rect, QGraphicsItem *parent = nullptr);

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

    // 圆角（四角独立）
    void setCornerRadius(qreal tl, qreal tr, qreal br, qreal bl);
    qreal cornerRadiusTopLeft() const     { return m_radiusTL; }
    qreal cornerRadiusTopRight() const    { return m_radiusTR; }
    qreal cornerRadiusBottomRight() const { return m_radiusBR; }
    qreal cornerRadiusBottomLeft() const  { return m_radiusBL; }

    // 圆角调节手柄
    QVector<QPointF> cornerRadiusHandlePositions() const override;
    // 由本地坐标点设置第 index 个角的圆角半径（沿角平分线投影）
    void setCornerRadiusFromPoint(int index, const QPointF &localPt);

    // 顶点编辑（4 角）
    QVector<QPointF> anchorPoints() const override;
    void setAnchorPoint(int index, const QPointF &pt) override;
    void setAnchorPoints(const QVector<QPointF> &points) override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QPolygonF m_vertices;   // 4 角顶点（本地坐标）
    QRectF m_rect;          // 包围盒（size/resize/对齐用）
    qreal m_radiusTL = 0.0, m_radiusTR = 0.0;
    qreal m_radiusBR = 0.0, m_radiusBL = 0.0;

    QPointF cornerBisector(int index) const;
    void setDefaultVertices(const QRectF &r);
    void updateRectFromVertices();
};

#endif // RECTSHAPE_H
