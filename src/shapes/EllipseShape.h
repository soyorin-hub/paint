#ifndef ELLIPSESHAPE_H
#define ELLIPSESHAPE_H

#include "ShapeBase.h"
#include <QRectF>

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

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QRectF m_rect;
};

#endif // ELLIPSESHAPE_H
