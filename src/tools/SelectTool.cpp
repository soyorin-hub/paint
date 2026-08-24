#include "SelectTool.h"
#include "canvas/CanvasScene.h"
#include "shapes/RectShape.h"
#include "shapes/EllipseShape.h"
#include "shapes/TriangleShape.h"
#include "shapes/DiamondShape.h"
#include "commands/TransformCommand.h"
#include "commands/LineCommand.h"
#include "commands/RotateCommand.h"
#include "commands/CornerRadiusCommand.h"
#include "commands/VertexCommand.h"
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
    qreal threshold = 10.0;

    // 圆角手柄优先（位于缩放手柄内侧）
    QVector<QPointF> radiusHandles = shape->cornerRadiusHandlePositions();
    for (int i = 0; i < radiusHandles.size(); ++i) {
        QPointF scenePt = shape->mapToScene(radiusHandles[i]);
        if (QLineF(scenePos, scenePt).length() < threshold)
            return static_cast<HandleType>(Handle_RadiusTopLeft + i);
    }

    QVector<QPointF> handles = shape->handlePositions();
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
    case Handle_RadiusTopLeft:
    case Handle_RadiusTopRight:
    case Handle_RadiusBottomRight:
    case Handle_RadiusBottomLeft:
        return Qt::SizeAllCursor;     // 圆角调节
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
            m_originalVertices = shape->anchorPoints();
            if (auto *rs = dynamic_cast<RectShape*>(shape)) {
                m_origRadiusTL = rs->cornerRadiusTopLeft();
                m_origRadiusTR = rs->cornerRadiusTopRight();
                m_origRadiusBR = rs->cornerRadiusBottomRight();
                m_origRadiusBL = rs->cornerRadiusBottomLeft();
            }
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
        } else if (m_activeHandle >= Handle_RadiusTopLeft && m_activeHandle <= Handle_RadiusBottomLeft) {
            // 圆角调节
            applyCornerRadius(shape, m_activeHandle, event->scenePos());
        } else {
            // 缩放：对边/对顶点保持不变（受尺子约束）
            QPointF clamped = scene->constrainToRuler(event->scenePos(), m_originalPos);
            applyResize(shape, m_activeHandle, clamped);
        }
        scene->setModified(true);
        return;
    }

    if (m_movingItem && !m_moveItems.isEmpty() && (event->buttons() & Qt::LeftButton)) {
        QPointF delta = event->scenePos() - m_lastScenePos;
        m_lastScenePos = event->scenePos();
        RulerGuide ruler = scene->ruler();
        for (auto *it : m_moveItems) {
            QPointF d = delta;
            if (ruler.visible) {
                QRectF box = it->sceneBoundingRect();
                if (ruler.orientation == Qt::Horizontal) {
                    qreal y = ruler.position;
                    if (box.bottom() <= y)       d.setY(qMin(d.y(), y - box.bottom()));
                    else if (box.top() >= y)     d.setY(qMax(d.y(), y - box.top()));
                } else {
                    qreal x = ruler.position;
                    if (box.right() <= x)        d.setX(qMin(d.x(), x - box.right()));
                    else if (box.left() >= x)    d.setX(qMax(d.x(), x - box.left()));
                }
            }
            it->moveBy(d.x(), d.y());
        }
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
            } else if (m_activeHandle >= Handle_RadiusTopLeft && m_activeHandle <= Handle_RadiusBottomLeft) {
                auto *rs = dynamic_cast<RectShape*>(shape);
                if (rs &&
                    (rs->cornerRadiusTopLeft() != m_origRadiusTL
                     || rs->cornerRadiusTopRight() != m_origRadiusTR
                     || rs->cornerRadiusBottomRight() != m_origRadiusBR
                     || rs->cornerRadiusBottomLeft() != m_origRadiusBL)) {
                    scene->pushUndoCommand(new CornerRadiusCommand(
                        rs, m_origRadiusTL, m_origRadiusTR, m_origRadiusBR, m_origRadiusBL,
                        rs->cornerRadiusTopLeft(), rs->cornerRadiusTopRight(),
                        rs->cornerRadiusBottomRight(), rs->cornerRadiusBottomLeft(),
                        tr("调节圆角")));
                }
            } else if (!shape->size().isEmpty()) {
                // 多边形/椭圆缩放：保存顶点，保留变形
                if (shape->anchorPoints() != m_originalVertices)
                    scene->pushUndoCommand(new VertexCommand(
                        shape, m_originalVertices, shape->anchorPoints(), tr("缩放图形")));
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

    auto *rectShape = dynamic_cast<RectShape*>(shape);
    auto *ellipseShape = dynamic_cast<EllipseShape*>(shape);
    auto *triangleShape = dynamic_cast<TriangleShape*>(shape);
    auto *diamondShape = dynamic_cast<DiamondShape*>(shape);

    // ===== 多边形/椭圆：按比例缩放顶点，保留变形 =====
    if (rectShape || ellipseShape || triangleShape || diamondShape) {
        if (m_originalVertices.isEmpty()) return;
        QRectF bounds;
        bool first = true;
        for (const QPointF &v : m_originalVertices) {
            if (first) { bounds = QRectF(v, v); first = false; }
            else {
                bounds.setLeft(qMin(bounds.left(), v.x()));
                bounds.setRight(qMax(bounds.right(), v.x()));
                bounds.setTop(qMin(bounds.top(), v.y()));
                bounds.setBottom(qMax(bounds.bottom(), v.y()));
            }
        }
        if (bounds.width() < 1e-6) bounds.setWidth(1.0);
        if (bounds.height() < 1e-6) bounds.setHeight(1.0);

        QPointF anchor;
        bool scaleX = true, scaleY = true;
        switch (handle) {
        case Handle_TopLeft:     anchor = bounds.bottomRight(); break;
        case Handle_Top:         anchor = QPointF(bounds.center().x(), bounds.bottom()); scaleX = false; break;
        case Handle_TopRight:    anchor = bounds.bottomLeft(); break;
        case Handle_Right:       anchor = QPointF(bounds.left(), bounds.center().y()); scaleY = false; break;
        case Handle_BottomRight: anchor = bounds.topLeft(); break;
        case Handle_Bottom:      anchor = QPointF(bounds.center().x(), bounds.top()); scaleX = false; break;
        case Handle_BottomLeft:  anchor = bounds.topRight(); break;
        case Handle_Left:        anchor = QPointF(bounds.right(), bounds.center().y()); scaleY = false; break;
        default: return;
        }

        qreal minW = 5.0, minH = 5.0;
        qreal sx = 1.0, sy = 1.0;
        if (scaleX) {
            qreal movingX = (handle == Handle_TopLeft || handle == Handle_BottomLeft)
                                ? bounds.left() : bounds.right();
            if (qAbs(movingX - anchor.x()) > 1e-6)
                sx = (localPt.x() - anchor.x()) / (movingX - anchor.x());
            sx = qMax(sx, minW / bounds.width());
        }
        if (scaleY) {
            qreal movingY = (handle == Handle_TopLeft || handle == Handle_TopRight)
                                ? bounds.top() : bounds.bottom();
            if (qAbs(movingY - anchor.y()) > 1e-6)
                sy = (localPt.y() - anchor.y()) / (movingY - anchor.y());
            sy = qMax(sy, minH / bounds.height());
        }

        QVector<QPointF> verts = m_originalVertices;
        for (QPointF &v : verts) {
            v.setX(anchor.x() + (v.x() - anchor.x()) * sx);
            v.setY(anchor.y() + (v.y() - anchor.y()) * sy);
        }
        shape->setAnchorPoints(verts);
        return;
    }

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

    qreal cosFwd = std::cos(rotRad), sinFwd = std::sin(rotRad);

    // 线段/自由画笔：以对边/对顶点为基准进行缩放（transform）
    {
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

void SelectTool::applyCornerRadius(ShapeBase *shape, HandleType handle, const QPointF &newScenePos)
{
    auto *rs = dynamic_cast<RectShape*>(shape);
    if (!rs) return;

    int idx = -1;
    switch (handle) {
    case Handle_RadiusTopLeft:     idx = 0; break;
    case Handle_RadiusTopRight:    idx = 1; break;
    case Handle_RadiusBottomRight: idx = 2; break;
    case Handle_RadiusBottomLeft:  idx = 3; break;
    default: return;
    }

    // 圆角拖拽不改变 pos()，可直接用 mapFromScene 得到本地坐标（正确处理旋转）
    QPointF localPt = rs->mapFromScene(newScenePos);
    rs->setCornerRadiusFromPoint(idx, localPt);
}

// 端点手柄拖拽：index 0 = 起点，1 = 中心（弯折），2 = 终点
void SelectTool::dragLineEndpoint(ShapeBase *shape, int handleIndex, const QPointF &scenePos)
{
    shape->setLinePoint(handleIndex, shape->mapFromScene(scenePos));
}
