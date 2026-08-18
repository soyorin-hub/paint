#include "ShapeBase.h"
#include "RectShape.h"
#include "EllipseShape.h"
#include "LineShape.h"
#include "TriangleShape.h"
#include "DiamondShape.h"
#include "ArrowShape.h"
#include "FreehandShape.h"
#include "TextShape.h"

#include <QPainter>
#include <cmath>

ShapeBase::ShapeBase(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
    setFlags(QGraphicsItem::ItemIsSelectable
           | QGraphicsItem::ItemIsMovable
           | QGraphicsItem::ItemIsFocusable);
    setAcceptHoverEvents(true);
}

void ShapeBase::setShapeStyle(const ShapeStyle &style)
{
    m_style = style;
    update();
}

void ShapeBase::setShapeName(const QString &name)
{
    m_shapeName = name;
}

QRectF ShapeBase::selectionBoundingRect() const
{
    return boundingRect();
}

bool ShapeBase::contains(const QPointF &point) const
{
    return shape().contains(point);
}

// ===== 手柄 =====

QVector<QPointF> ShapeBase::handlePositions() const
{
    QRectF br = boundingRect();
    // 排除 padding，使用实际内容区域
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    QRectF r = br.adjusted(pad, pad, -pad, -pad);

    if (r.width() < 10) r.setWidth(10);
    if (r.height() < 10) r.setHeight(10);

    qreal hw = r.width() / 2.0;
    qreal hh = r.height() / 2.0;

    return {
        r.topLeft(),                          // 0:TopLeft
        QPointF(r.left() + hw, r.top()),      // 1:Top
        r.topRight(),                         // 2:TopRight
        QPointF(r.right(), r.top() + hh),     // 3:Right
        r.bottomRight(),                      // 4:BottomRight
        QPointF(r.left() + hw, r.bottom()),   // 5:Bottom
        r.bottomLeft(),                       // 6:BottomLeft
        QPointF(r.left(), r.top() + hh),      // 7:Left
        QPointF(r.left() + hw, r.top() - 14)  // 8:Rotate
    };
}

void ShapeBase::paintHandles(QPainter *painter, const QRectF &bodyRect,
                              bool showRotate) const
{
    Q_UNUSED(bodyRect)

    QVector<QPointF> pts = handlePositions();
    qreal hs = 8.0; // handle size

    // 旋转手柄的连接线
    if (showRotate && pts.size() > Handle_Rotate) {
        QPointF topCenter = pts[Handle_Top];
        QPointF rotPt = pts[Handle_Rotate];
        painter->setPen(QPen(QColor(0, 100, 200), 1.0, Qt::DotLine));
        painter->drawLine(topCenter, rotPt);
    }

    // 画手柄
    for (int i = 0; i < pts.size(); ++i) {
        if (i == Handle_Rotate && !showRotate) continue;

        QPointF pt = pts[i];
        QColor fill, stroke;

        if (i == Handle_Rotate) {
            // 旋转手柄：绿色圆形，更大
            stroke = QColor(0, 150, 50);
            fill   = QColor(100, 255, 100);
            painter->setPen(QPen(stroke, 2.0));
            painter->setBrush(fill);
            painter->drawEllipse(pt, hs * 0.8, hs * 0.8);
        } else {
            // 缩放手柄：蓝色方块
            stroke = QColor(0, 100, 200);
            fill   = Qt::white;
            painter->setPen(QPen(stroke, 1.5));
            painter->setBrush(fill);
            painter->drawRect(QRectF(pt.x() - hs / 2.0, pt.y() - hs / 2.0, hs, hs));
        }
    }
}

// ===== 旋转 =====

void ShapeBase::setRotationAngle(qreal angle)
{
    // 设置旋转原点为虚线框（内容区域）中心
    QRectF br = boundingRect();
    qreal pad = m_style.strokeWidth / 2.0 + 2.0;
    QRectF contentRect = br.adjusted(pad, pad, -pad, -pad);
    setTransformOriginPoint(contentRect.center());

    m_rotation = angle;
    setRotation(m_rotation);
    update();
}

// ===== 序列化 =====

QJsonObject ShapeBase::toJson() const
{
    QJsonObject obj;
    obj["pos"]   = QString("%1,%2").arg(pos().x()).arg(pos().y());
    obj["style"] = m_style.toJson();
    if (!qFuzzyIsNull(m_rotation))
        obj["rotation"] = m_rotation;
    if (m_groupId >= 0)
        obj["group"] = m_groupId;
    return obj;
}

void ShapeBase::fromJson(const QJsonObject &obj)
{
    if (obj.contains("pos")) {
        QStringList parts = obj["pos"].toString().split(',');
        if (parts.size() == 2) {
            setPos(parts[0].toDouble(), parts[1].toDouble());
        }
    }
    if (obj.contains("style")) {
        m_style.fromJson(obj["style"].toObject());
    }
    m_rotation = obj["rotation"].toDouble(0.0);
    setRotation(m_rotation);
    m_groupId = obj["group"].toInteger(-1);
}

ShapeBase *ShapeBase::createFromJson(const QJsonObject &obj)
{
    QString type = obj["type"].toString();
    ShapeBase *shape = nullptr;

    if (type == "RectShape") {
        shape = new RectShape();
    } else if (type == "EllipseShape") {
        shape = new EllipseShape();
    } else if (type == "LineShape") {
        shape = new LineShape();
    } else if (type == "TriangleShape") {
        shape = new TriangleShape();
    } else if (type == "DiamondShape") {
        shape = new DiamondShape();
    } else if (type == "ArrowShape") {
        shape = new ArrowShape();
    } else if (type == "FreehandShape") {
        shape = new FreehandShape();
    } else if (type == "TextShape") {
        shape = new TextShape();
    }

    if (shape) {
        shape->fromJson(obj);
    }
    return shape;
}
