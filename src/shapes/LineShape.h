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

    QLineF line() const { return m_line; }
    void setLine(const QLineF &line);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QLineF m_line;
};

#endif // LINESHAPE_H
