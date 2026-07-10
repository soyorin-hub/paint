#ifndef SHAPEBASE_H
#define SHAPEBASE_H

#include <QGraphicsItem>
#include <QJsonObject>
#include <QVector>
#include "style/ShapeStyle.h"

// 手柄类型
enum HandleType {
    Handle_None = -1,
    Handle_TopLeft, Handle_Top, Handle_TopRight,
    Handle_Right, Handle_BottomRight, Handle_Bottom,
    Handle_BottomLeft, Handle_Left,
    Handle_Rotate,
    Handle_Count
};

class ShapeBase : public QGraphicsItem
{
public:
    enum { Type = QGraphicsItem::UserType + 1 };
    int type() const override { return Type; }

    explicit ShapeBase(QGraphicsItem *parent = nullptr);

    // ===== 样式 =====
    void setShapeStyle(const ShapeStyle &style);
    ShapeStyle shapeStyle() const { return m_style; }

    // ===== 几何 =====
    virtual QRectF boundingRect() const = 0;
    // 选中时的包围盒（包含手柄空间，用于正确裁剪）
    virtual QRectF selectionBoundingRect() const;
    virtual QPainterPath shape() const { return QGraphicsItem::shape(); }
    virtual bool contains(const QPointF &point) const override;

    // ===== 选中手柄 =====
    // 返回 9 个手柄在本地坐标系中的位置
    virtual QVector<QPointF> handlePositions() const;
    // 绘制所有手柄
    void paintHandles(QPainter *painter, const QRectF &bodyRect,
                      bool showRotate = true) const;

    // ===== 编辑手柄 =====
    virtual void setP2(const QPointF &p2) { Q_UNUSED(p2) }
    virtual bool isFinished() const { return true; }

    // ===== 变换 =====
    qreal rotationAngle() const { return m_rotation; }
    void setRotationAngle(qreal angle);

    // ===== 序列化 =====
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject &obj);

    // 工厂方法
    static ShapeBase *createFromJson(const QJsonObject &obj);

    // 手柄需要的顶部额外空间
    static constexpr qreal HANDLE_TOP_CLEARANCE = 18.0;

protected:
    void updateStylePen();

    ShapeStyle m_style;
    bool m_finished = true;
    qreal m_rotation = 0.0;   // 旋转角度（度）
};

#endif // SHAPEBASE_H
