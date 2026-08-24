#ifndef LINESHAPE_H
#define LINESHAPE_H

#include "ShapeBase.h"
#include <QLineF>

class LineShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 4 };
    int type() const override { return Type; }

    explicit LineShape(QGraphicsItem *parent = nullptr);
    LineShape(const QLineF &line, QGraphicsItem *parent = nullptr);

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

    // 顶点编辑
    QVector<QPointF> anchorPoints() const override;
    void setAnchorPoint(int index, const QPointF &pt) override;
    void setAnchorPoints(const QVector<QPointF> &points) override;
    QPainterPath outlinePath() const override;

    QLineF line() const { return m_line; }
    void setLine(const QLineF &line);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QLineF m_line;
    QPointF m_center;   // 可拖动的中心顶点（初始为中点，弯折后独立）
};

#endif // LINESHAPE_H
