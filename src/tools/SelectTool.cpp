#include "SelectTool.h"
#include "canvas/CanvasScene.h"
#include "shapes/RectShape.h"
#include "shapes/EllipseShape.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QCursor>
#include <cmath>

SelectTool::SelectTool(QObject *parent)
    : ToolBase(parent)
{
}

// 检测鼠标位置对应的手柄
HandleType SelectTool::handleAt(const QPointF &scenePos, ShapeBase *shape) const
{
    QVector<QPointF> handles = shape->handlePositions();
    qreal threshold = 10.0;

    for (int i = 0; i < handles.size(); ++i) {
        QPointF localPt = handles[i];
        QPointF scenePt = shape->mapToScene(localPt);
        if (QLineF(scenePos, scenePt).length() < threshold) {
            return static_cast<HandleType>(i);
        }
    }
    return Handle_None;
}

void SelectTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_dragStartPos = event->scenePos();
    m_lastScenePos = event->scenePos();

    // 先检查已选中的图形是否有手柄被点击
    QList<QGraphicsItem*> selected = scene->selectedItems();
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (!shape) continue;

        HandleType h = handleAt(event->scenePos(), shape);
        if (h != Handle_None) {
            m_activeItem = shape;
            m_activeHandle = h;
            m_handleDragging = true;
            m_movingItem = false;
            m_originalPos = shape->pos();
            m_originalRect = shape->boundingRect();
            m_originalRect = m_originalRect.adjusted(
                shape->shapeStyle().strokeWidth / 2.0 + 2.0,
                shape->shapeStyle().strokeWidth / 2.0 + 2.0,
                -(shape->shapeStyle().strokeWidth / 2.0 + 2.0),
                -(shape->shapeStyle().strokeWidth / 2.0 + 2.0));
            m_originalRotate = shape->rotationAngle();
            return;
        }
    }

    // 没有手柄 → 正常选择/移动
    QGraphicsItem *item = scene->itemAt(event->scenePos(), QTransform());
    if (item) {
        if (!(event->modifiers() & Qt::ControlModifier)) {
            scene->clearSelection();
        }
        item->setSelected(true);
        m_activeItem = item;
        m_movingItem = true;
        m_handleDragging = false;
    } else {
        scene->clearSelection();
    }
}

void SelectTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (m_handleDragging && m_activeItem) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(m_activeItem);
        if (!shape) return;

        if (m_activeHandle == Handle_Rotate) {
            // 旋转：计算角度
            QPointF center = shape->mapToScene(shape->boundingRect().center());
            qreal angle1 = std::atan2(m_dragStartPos.y() - center.y(),
                                       m_dragStartPos.x() - center.x());
            qreal angle2 = std::atan2(event->scenePos().y() - center.y(),
                                       event->scenePos().x() - center.x());
            qreal deltaDeg = (angle2 - angle1) * 180.0 / M_PI;
            shape->setRotationAngle(m_originalRotate + deltaDeg);
        } else {
            // 缩放
            applyResize(shape, m_activeHandle, event->scenePos());
        }
        scene->setModified(true);
        return;
    }

    if (m_movingItem && m_activeItem && (event->buttons() & Qt::LeftButton)) {
        QPointF delta = event->scenePos() - m_lastScenePos;
        m_lastScenePos = event->scenePos();
        m_activeItem->moveBy(delta.x(), delta.y());
        scene->setModified(true);
        return;
    }
}

void SelectTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event)

    if (m_handleDragging) {
        m_handleDragging = false;
        m_activeItem = nullptr;
        m_activeHandle = Handle_None;
        scene->setModified(true);
        return;
    }

    if (m_movingItem) {
        m_movingItem = false;
        m_activeItem = nullptr;
        scene->setModified(true);
    }
}

void SelectTool::applyResize(ShapeBase *shape, HandleType handle, const QPointF &newScenePos)
{
    // 将新位置转换到图形本地坐标
    QPointF localPt = shape->mapFromScene(newScenePos);
    QRectF r = m_originalRect;

    // 保持宽高最小限制
    qreal minW = 5.0, minH = 5.0;

    switch (handle) {
    case Handle_TopLeft:
        r.setTopLeft(localPt);
        break;
    case Handle_TopRight:
        r.setTopRight(localPt);
        break;
    case Handle_BottomRight:
        r.setBottomRight(localPt);
        break;
    case Handle_BottomLeft:
        r.setBottomLeft(localPt);
        break;
    case Handle_Top:
        r.setTop(localPt.y());
        break;
    case Handle_Bottom:
        r.setBottom(localPt.y());
        break;
    case Handle_Left:
        r.setLeft(localPt.x());
        break;
    case Handle_Right:
        r.setRight(localPt.x());
        break;
    default:
        return;
    }

    r = r.normalized();
    if (r.width() < minW) r.setWidth(minW);
    if (r.height() < minH) r.setHeight(minH);

    // 根据类型更新形状
    auto *rectShape = dynamic_cast<RectShape*>(shape);
    auto *ellipseShape = dynamic_cast<EllipseShape*>(shape);

    if (rectShape) {
        QPointF delta = r.topLeft() - m_originalRect.topLeft();
        rectShape->setPos(shape->pos() + delta);
        rectShape->setRect(QRectF(QPointF(0, 0), r.size()));
    } else if (ellipseShape) {
        QPointF delta = r.topLeft() - m_originalRect.topLeft();
        ellipseShape->setPos(shape->pos() + delta);
        ellipseShape->setRect(QRectF(QPointF(0, 0), r.size()));
    } else {
        // 对于 LineShape 和 FreehandShape，通过缩放变换
        QPointF delta = r.topLeft() - m_originalRect.topLeft();
        shape->setPos(shape->pos() + delta);
        qreal sx = r.width() / qMax(m_originalRect.width(), 1.0);
        qreal sy = r.height() / qMax(m_originalRect.height(), 1.0);
        shape->setTransform(QTransform().scale(sx, sy));
    }
}
