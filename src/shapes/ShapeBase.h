#ifndef SHAPEBASE_H
#define SHAPEBASE_H

#include <QGraphicsItem>
#include <QJsonObject>
#include <QVector>
#include <QLineF>
#include "style/ShapeStyle.h"

// 手柄类型
enum HandleType {
    Handle_None = -1,
    Handle_TopLeft, Handle_Top, Handle_TopRight,
    Handle_Right, Handle_BottomRight, Handle_Bottom,
    Handle_BottomLeft, Handle_Left,
    Handle_Rotate,
    Handle_RadiusTopLeft, Handle_RadiusTopRight,
    Handle_RadiusBottomRight, Handle_RadiusBottomLeft,
    Handle_Anchor,
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

    // 几何尺寸（对基于矩形的图形有效，其余返回空 QSizeF）
    virtual QSizeF size() const { return QSizeF(); }
    // 设置几何尺寸（对基于矩形的图形有效，其余默认无操作）
    virtual void setSize(const QSizeF &size) { Q_UNUSED(size) }
    // 内容包围盒（不含描边/手柄余量），用于对齐；默认退回 boundingRect
    virtual QRectF contentRect() const { return boundingRect(); }

    // ===== 线段类图形（线段/箭头）的端点手柄 =====
    // 返回 true 表示使用「两端点 + 中心点」手柄，而非默认的 8 向缩放框
    virtual bool usesEndpointHandles() const { return false; }
    // 顶点访问：0 = 起点，1 = 中心（可弯折），2 = 终点
    virtual QPointF linePoint(int index) const { Q_UNUSED(index) return QPointF(); }
    virtual void setLinePoint(int index, const QPointF &pt) { Q_UNUSED(index) Q_UNUSED(pt) }

    // ===== 名称 =====
    QString shapeName() const { return m_shapeName; }
    void setShapeName(const QString &name);

    // ===== 编组 =====
    qint64 groupId() const { return m_groupId; }
    void setGroupId(qint64 id) { m_groupId = id; }

    // ===== 选中手柄 =====
    // 返回 9 个手柄在本地坐标系中的位置
    virtual QVector<QPointF> handlePositions() const;
    // 圆角调节手柄位置（仅支持圆角的图形，如矩形；默认无）
    virtual QVector<QPointF> cornerRadiusHandlePositions() const { return {}; }
    // 路径锚点（顶点）编辑：仅自由钢笔等路径图形有；默认无
    virtual QVector<QPointF> anchorPoints() const { return {}; }
    virtual void setAnchorPoint(int index, const QPointF &pt) { Q_UNUSED(index) Q_UNUSED(pt) }
    // 一次性设置全部锚点（撤销用）
    virtual void setAnchorPoints(const QVector<QPointF> &points) { Q_UNUSED(points) }
    // 直接选择状态
    void setDirectSelected(bool on);
    bool isDirectSelected() const { return m_directSelected; }
    // 悬停中的锚点（-1 表示无，悬停时该锚点会变大）
    void setHoveredAnchor(int idx);
    int hoveredAnchor() const { return m_hoveredAnchor; }
    // 直接选择时贴合的轮廓（细线，默认返回 shape()）
    virtual QPainterPath outlinePath() const { return shape(); }
    // 绘制直接选择高亮（贴边轮廓 + 顶点锚点）
    void paintDirectSelectionHighlights(QPainter *painter) const;
    // 绘制所有手柄
    void paintHandles(QPainter *painter, const QRectF &bodyRect,
                      bool showRotate = true) const;

    // ===== 编辑手柄 =====
    virtual void setP2(const QPointF &p2) { Q_UNUSED(p2) }
    virtual bool isFinished() const { return true; }
    virtual void setFinished(bool f) { Q_UNUSED(f) }

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
    ShapeStyle m_style;
    bool m_finished = true;
    qreal m_rotation = 0.0;   // 旋转角度（度）
    QString m_shapeName;       // 图形名称（用户可修改）
    qint64 m_groupId = -1;     // 编组 ID（-1 表示未编组）
    bool m_directSelected = false;  // 直接选择（顶点编辑）态
    int  m_hoveredAnchor = -1;      // 悬停的锚点索引
};

#endif // SHAPEBASE_H
