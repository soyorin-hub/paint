#include "SelectTool.h"
#include "canvas/CanvasScene.h"
#include "shapes/RectShape.h"
#include "shapes/EllipseShape.h"
#include "shapes/TriangleShape.h"
#include "shapes/DiamondShape.h"
#include "commands/TransformCommand.h"
#include "commands/LineCommand.h"
#include "commands/ResizeCommand.h"
#include "commands/RotateCommand.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsView>
#include <QPainterPath>
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

// 根据手柄类型返回对应的双向箭头光标
QCursor SelectTool::cursorForHandle(HandleType h)
{
    switch (h) {
    case Handle_TopLeft:
    case Handle_BottomRight:
        return Qt::SizeFDiagCursor;   // ↖↘ 斜45°双向箭头
    case Handle_TopRight:
    case Handle_BottomLeft:
        return Qt::SizeBDiagCursor;   // ↗↙ 斜45°双向箭头
    case Handle_Top:
    case Handle_Bottom:
        return Qt::SizeVerCursor;     // ↑↓ 垂直双向箭头
    case Handle_Left:
    case Handle_Right:
        return Qt::SizeHorCursor;     // ↔ 水平双向箭头
    case Handle_Rotate:
        return Qt::CrossCursor;       // 旋转光标
    default:
        return Qt::ArrowCursor;
    }
}

// 辅助：在 scene 关联的 viewport 上设置光标
void SelectTool::setViewCursor(CanvasScene *scene, const QCursor &cursor)
{
    if (!scene) return;
    const auto views = scene->views();
    for (auto *view : views) {
        view->viewport()->setCursor(cursor);
    }
}

// 根据鼠标位置更新视口光标（hover 手柄 / 可移动区域）
void SelectTool::updateCursor(CanvasScene *scene, const QPointF &scenePos)
{
    if (!scene) return;

    // 检查已选中的图形是否有手柄被 hover
    QList<QGraphicsItem*> selected = scene->selectedItems();
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (!shape) continue;

        HandleType h = handleAt(scenePos, shape);
        if (h != Handle_None) {
            if (shape->usesEndpointHandles())
                setViewCursor(scene, (h == Handle_Top) ? Qt::OpenHandCursor : Qt::CrossCursor);
            else
                setViewCursor(scene, cursorForHandle(h));
            return;
        }
    }

    // 检查是否 hover 在可选中的图形上（可移动范围）
    QGraphicsItem *item = scene->itemAt(scenePos, QTransform());
    if (item && dynamic_cast<ShapeBase*>(item)) {
        setViewCursor(scene, Qt::OpenHandCursor);
        return;
    }

    // 默认箭头
    setViewCursor(scene, Qt::ArrowCursor);
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
            m_originalTransform = shape->transform();
            m_originalRect = shape->boundingRect();
            m_originalRect = m_originalRect.adjusted(
                shape->shapeStyle().strokeWidth / 2.0 + 2.0,
                shape->shapeStyle().strokeWidth / 2.0 + 2.0,
                -(shape->shapeStyle().strokeWidth / 2.0 + 2.0),
                -(shape->shapeStyle().strokeWidth / 2.0 + 2.0));
            m_originalRotate = shape->rotationAngle();
            m_originalLine = QLineF(shape->linePoint(0), shape->linePoint(2));
            m_originalCenter = shape->linePoint(1);
            m_originalSize = shape->size();
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
        // 编组联动：选中同一组的其它图形
        if (auto *s = dynamic_cast<ShapeBase*>(item)) {
            qint64 gid = s->groupId();
            if (gid >= 0) {
                for (auto *it : scene->items()) {
                    auto *other = dynamic_cast<ShapeBase*>(it);
                    if (other && other->groupId() == gid)
                        other->setSelected(true);
                }
            }
        }
        m_activeItem = item;
        m_movingItem = true;
        m_handleDragging = false;
        // 记录所有选中项的原始位置/变换（多选/编组移动）
        m_moveItems = scene->selectedItems();
        m_moveOrigPos.clear();
        m_moveOrigXf.clear();
        for (auto *it : m_moveItems) {
            m_moveOrigPos.append(it->pos());
            m_moveOrigXf.append(it->transform());
        }
        // 长按拖动 → 小手抓取光标
        setViewCursor(scene, Qt::ClosedHandCursor);
    } else {
        // 空白处按下 → 开始框选
        m_marqueeSelecting = true;
        m_marqueeStart = event->scenePos();
        m_marqueeAdd = event->modifiers() & Qt::ControlModifier;
        m_activeItem = nullptr;
        m_movingItem = false;
        m_handleDragging = false;
        if (!m_marqueeAdd) scene->clearSelection();

        if (!m_marqueeRect) {
            m_marqueeRect = new QGraphicsRectItem();
            m_marqueeRect->setPen(QPen(QColor(0, 120, 215), 1, Qt::DashLine));
            m_marqueeRect->setBrush(QColor(0, 120, 215, 30));
            m_marqueeRect->setZValue(10000);
            m_marqueeRect->setFlag(QGraphicsItem::ItemIsSelectable, false);
            scene->addItem(m_marqueeRect);
        }
        m_marqueeRect->setRect(QRectF(m_marqueeStart, m_marqueeStart));
        m_marqueeRect->show();
    }
}

void SelectTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (m_marqueeSelecting) {
        QRectF rect = QRectF(m_marqueeStart, event->scenePos()).normalized();
        if (m_marqueeRect) m_marqueeRect->setRect(rect);

        QPainterPath path;
        path.addRect(rect);
        QList<QGraphicsItem*> inside = scene->items(path, Qt::IntersectsItemShape);
        if (m_marqueeAdd) {
            for (auto *it : inside)
                if (dynamic_cast<ShapeBase*>(it)) it->setSelected(true);
        } else {
            for (auto *it : scene->items())
                if (dynamic_cast<ShapeBase*>(it))
                    it->setSelected(inside.contains(it));
        }
        return;
    }

    if (m_handleDragging && m_activeItem) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(m_activeItem);
        if (!shape) return;

        if (shape->usesEndpointHandles()) {
            dragLineEndpoint(shape, static_cast<int>(m_activeHandle), event->scenePos());
        } else if (m_activeHandle == Handle_Rotate) {
            // 旋转：以虚线框中心点为基准
            QVector<QPointF> handles = shape->handlePositions();
            QPointF localCenter = (handles[Handle_TopLeft] + handles[Handle_BottomRight]) / 2.0;
            QPointF center = shape->mapToScene(localCenter);
            qreal angle1 = std::atan2(m_dragStartPos.y() - center.y(),
                                       m_dragStartPos.x() - center.x());
            qreal angle2 = std::atan2(event->scenePos().y() - center.y(),
                                       event->scenePos().x() - center.x());
            qreal deltaDeg = (angle2 - angle1) * 180.0 / M_PI;
            shape->setRotationAngle(m_originalRotate + deltaDeg);
        } else {
            // 缩放：对边/对顶点保持不变
            applyResize(shape, m_activeHandle, event->scenePos());
        }
        scene->setModified(true);
        return;
    }

    if (m_movingItem && !m_moveItems.isEmpty() && (event->buttons() & Qt::LeftButton)) {
        QPointF delta = event->scenePos() - m_lastScenePos;
        m_lastScenePos = event->scenePos();
        for (auto *it : m_moveItems)
            it->moveBy(delta.x(), delta.y());
        scene->setModified(true);
        // 拖动过程中保持小手光标
        setViewCursor(scene, Qt::ClosedHandCursor);
        return;
    }

    // 没有拖拽操作时，根据 hover 位置更新光标
    updateCursor(scene, event->scenePos());
}

void SelectTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event)

    if (m_marqueeSelecting) {
        m_marqueeSelecting = false;
        if (m_marqueeRect) {
            scene->removeItem(m_marqueeRect);
            delete m_marqueeRect;
            m_marqueeRect = nullptr;
        }
        setViewCursor(scene, Qt::ArrowCursor);
        return;
    }

    if (m_handleDragging) {
        m_handleDragging = false;
        // 记录缩放/旋转/端点编辑操作
        ShapeBase *shape = dynamic_cast<ShapeBase*>(m_activeItem);
        if (shape && m_activeItem) {
            if (shape->usesEndpointHandles()) {
                QLineF newLine(shape->linePoint(0), shape->linePoint(2));
                QPointF newCenter = shape->linePoint(1);
                if (m_originalLine != newLine || m_originalCenter != newCenter)
                    scene->pushUndoCommand(new LineCommand(
                        shape, m_originalLine, m_originalCenter,
                        newLine, newCenter, tr("编辑线段")));
            } else if (m_activeHandle == Handle_Rotate) {
                if (shape->rotationAngle() != m_originalRotate)
                    scene->pushUndoCommand(new RotateCommand(
                        shape, m_originalRotate, shape->rotationAngle()));
            } else if (!shape->size().isEmpty()) {
                // 矩形类缩放：同时记录位置与尺寸
                scene->pushUndoCommand(new ResizeCommand(
                    shape, m_originalPos, m_originalSize,
                    shape->pos(), shape->size(), tr("缩放图形")));
            } else {
                // 自由画笔/文字：缩放体现在 transform 中
                scene->pushUndoCommand(new TransformCommand(
                    m_activeItem, m_originalPos, m_originalTransform,
                    m_activeItem->pos(), m_activeItem->transform(), tr("缩放图形")));
            }
        }
        m_activeItem = nullptr;
        m_activeHandle = Handle_None;
        scene->setModified(true);
        setViewCursor(scene, Qt::ArrowCursor);
        return;
    }

    if (m_movingItem) {
        m_movingItem = false;
        // 多选/编组移动：为每个移动过的项记录变换
        bool anyMoved = false;
        for (int i = 0; i < m_moveItems.size(); ++i) {
            if (m_moveOrigPos[i] != m_moveItems[i]->pos()
                || m_moveOrigXf[i] != m_moveItems[i]->transform()) {
                anyMoved = true;
                break;
            }
        }
        if (anyMoved) {
            scene->beginUndoMacro(tr("移动图形"));
            for (int i = 0; i < m_moveItems.size(); ++i) {
                QGraphicsItem *it = m_moveItems[i];
                if (m_moveOrigPos[i] != it->pos() || m_moveOrigXf[i] != it->transform()) {
                    scene->pushUndoCommand(new TransformCommand(
                        it, m_moveOrigPos[i], m_moveOrigXf[i],
                        it->pos(), it->transform()));
                }
            }
            scene->endUndoMacro();
        }
        m_moveItems.clear();
        m_moveOrigPos.clear();
        m_moveOrigXf.clear();
        m_activeItem = nullptr;
        scene->setModified(true);
        setViewCursor(scene, Qt::ArrowCursor);
        return;
    }
}

void SelectTool::applyResize(ShapeBase *shape, HandleType handle, const QPointF &newScenePos)
{
    // 旋转角度（弧度），用于坐标映射
    qreal rotRad = m_originalRotate * M_PI / 180.0;
    qreal cosR = std::cos(-rotRad), sinR = std::sin(-rotRad);

    // 将场景坐标映射到原始本地坐标系（以 m_originalPos 为基准）
    // 不能使用 shape->mapFromScene()，因为 shape->pos() 在拖拽过程中会被 setPos 改变
    QPointF shifted = newScenePos - m_originalPos;
    QPointF localPt(shifted.x() * cosR - shifted.y() * sinR,
                    shifted.x() * sinR + shifted.y() * cosR);

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

    // 位置 delta（本地坐标）→ 转回场景坐标
    QPointF localDelta = r.topLeft() - m_originalRect.topLeft();
    qreal cosFwd = std::cos(rotRad), sinFwd = std::sin(rotRad);
    QPointF sceneDelta(localDelta.x() * cosFwd - localDelta.y() * sinFwd,
                       localDelta.x() * sinFwd + localDelta.y() * cosFwd);

    // 根据类型更新形状
    auto *rectShape = dynamic_cast<RectShape*>(shape);
    auto *ellipseShape = dynamic_cast<EllipseShape*>(shape);
    auto *triangleShape = dynamic_cast<TriangleShape*>(shape);
    auto *diamondShape = dynamic_cast<DiamondShape*>(shape);

    if (rectShape || ellipseShape || triangleShape || diamondShape) {
        if (rectShape) {
            rectShape->setPos(m_originalPos + sceneDelta);
            rectShape->setRect(QRectF(QPointF(0, 0), r.size()));
        } else if (ellipseShape) {
            ellipseShape->setPos(m_originalPos + sceneDelta);
            ellipseShape->setRect(QRectF(QPointF(0, 0), r.size()));
        } else if (triangleShape) {
            triangleShape->setPos(m_originalPos + sceneDelta);
            triangleShape->setRect(QRectF(QPointF(0, 0), r.size()));
        } else if (diamondShape) {
            diamondShape->setPos(m_originalPos + sceneDelta);
            diamondShape->setRect(QRectF(QPointF(0, 0), r.size()));
        }
    } else {
        // LineShape / FreehandShape：以对边/对顶点为基准进行缩放
        QPointF anchor;
        bool scaleX = true, scaleY = true;

        switch (handle) {
        case Handle_TopLeft:
            anchor = m_originalRect.bottomRight();
            break;
        case Handle_Top:
            anchor = QPointF(m_originalRect.center().x(), m_originalRect.bottom());
            scaleX = false;
            break;
        case Handle_TopRight:
            anchor = m_originalRect.bottomLeft();
            break;
        case Handle_Right:
            anchor = QPointF(m_originalRect.left(), m_originalRect.center().y());
            scaleY = false;
            break;
        case Handle_BottomRight:
            anchor = m_originalRect.topLeft();
            break;
        case Handle_Bottom:
            anchor = QPointF(m_originalRect.center().x(), m_originalRect.top());
            scaleX = false;
            break;
        case Handle_BottomLeft:
            anchor = m_originalRect.topRight();
            break;
        case Handle_Left:
            anchor = QPointF(m_originalRect.right(), m_originalRect.center().y());
            scaleY = false;
            break;
        default:
            return;
        }

        qreal sx = scaleX ? r.width()  / qMax(m_originalRect.width(),  1.0) : 1.0;
        qreal sy = scaleY ? r.height() / qMax(m_originalRect.height(), 1.0) : 1.0;

        // 位置补偿：使锚点在场景坐标中保持不变
        QPointF localPosDelta = anchor - QPointF(anchor.x() * sx, anchor.y() * sy);
        QPointF scenePosDelta(localPosDelta.x() * cosFwd - localPosDelta.y() * sinFwd,
                              localPosDelta.x() * sinFwd + localPosDelta.y() * cosFwd);
        shape->setPos(m_originalPos + scenePosDelta);
        shape->setTransform(QTransform().scale(sx, sy));
    }
}

// 端点手柄拖拽：index 0 = 起点，1 = 中心（弯折），2 = 终点
void SelectTool::dragLineEndpoint(ShapeBase *shape, int handleIndex, const QPointF &scenePos)
{
    shape->setLinePoint(handleIndex, shape->mapFromScene(scenePos));
}
