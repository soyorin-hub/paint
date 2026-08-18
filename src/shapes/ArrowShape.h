#ifndef ARROWSHAPE_H
#define ARROWSHAPE_H

#include "ShapeBase.h"
#include <QLineF>

class ArrowShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 9 };
    int type() const override { return Type; }

    explicit ArrowShape(QGraphicsItem *parent = nullptr);
    ArrowShape(const QLineF &line, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    void setP2(const QPointF &p2) override;
    bool isFinished() const override { return m_finished; }
    void setFinished(bool f) { m_finished = f; }

    // 端点手柄：起点 + 中心 + 终点
    QVector<QPointF> handlePositions() const override;
    bool usesEndpointHandles() const override { return true; }
    QPointF linePoint(int index) const override;
    void setLinePoint(int index, const QPointF &pt) override;

    QLineF line() const { return m_line; }
    void setLine(const QLineF &line);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QLineF m_line;
    QPointF m_center;   // 可拖动的中心顶点（初始为中点，弯折后独立）
};

#endif // ARROWSHAPE_H
