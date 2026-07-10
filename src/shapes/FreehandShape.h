#ifndef FREEHANDSHAPE_H
#define FREEHANDSHAPE_H

#include "ShapeBase.h"
#include <QPainterPath>
#include <QPointF>
#include <QVector>

class FreehandShape : public ShapeBase
{
public:
    enum { Type = QGraphicsItem::UserType + 5 };
    int type() const override { return Type; }

    explicit FreehandShape(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    // 自由绘制：逐点追加
    void addPoint(const QPointF &point);
    void clearPath();

    bool isFinished() const override { return m_finished; }
    void setFinished(bool f) { m_finished = f; }

    const QPainterPath &path() const { return m_path; }
    void setPath(const QPainterPath &path);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &obj) override;

private:
    void rebuildPath();

    QVector<QPointF> m_points;
    QPainterPath m_path;
    QPointF m_lastPoint;
    QRectF m_cachedBoundingRect;
};

#endif // FREEHANDSHAPE_H
