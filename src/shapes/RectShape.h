#ifndef RECTSHAPE_H
#define RECTSHAPE_H

#include "ShapeBase.h"
#include <QRectF>

class RectShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 2 };
    int type() const override { return Type; }

    explicit RectShape(QGraphicsItem *parent = nullptr);
    RectShape(const QRectF &rect, QGraphicsItem *parent = nullptr);

    // 几何
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    // 创建时拖拽
    void setP2(const QPointF &p2) override;
    bool isFinished() const override { return m_finished; }
    void setFinished(bool f) { m_finished = f; }

    // 获取/设置矩形
    QRectF rect() const { return m_rect; }
    void setRect(const QRectF &rect);

    // 圆角
    void setCornerRadius(qreal r) { m_cornerRadius = r; update(); }
    qreal cornerRadius() const { return m_cornerRadius; }

    // 序列化
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    QRectF m_rect;
    qreal  m_cornerRadius = 0.0;
};

#endif // RECTSHAPE_H
