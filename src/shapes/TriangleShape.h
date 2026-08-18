#ifndef TRIANGLESHAPE_H
#define TRIANGLESHAPE_H

#include "ShapeBase.h"
#include <QRectF>

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

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QRectF m_rect;
};

#endif // TRIANGLESHAPE_H
