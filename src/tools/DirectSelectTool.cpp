#include "DirectSelectTool.h"
#include "canvas/CanvasScene.h"
#include "commands/VertexCommand.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QLineF>

DirectSelectTool::DirectSelectTool(QObject *parent)
    : ToolBase(parent)
{
}

int DirectSelectTool::anchorIndexAt(const QPointF &scenePos, ShapeBase *shape) const
{
    QVector<QPointF> anchors = shape->anchorPoints();
    qreal threshold = 18.0;
    for (int i = 0; i < anchors.size(); ++i) {
        if (QLineF(scenePos, shape->mapToScene(anchors[i])).length() < threshold)
            return i;
    }
    return -1;
}

void DirectSelectTool::deactivated()
{
    if (m_activeShape) {
        m_activeShape->setDirectSelected(false);
        m_activeShape = nullptr;
    }
    m_anchorDragging = false;
    m_activeAnchor = -1;
}

void DirectSelectTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_anchorDragging = false;
    m_activeAnchor = -1;

    QGraphicsItem *item = scene->itemAt(event->scenePos(), QTransform());
    ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
    // 锁定图层上的图形不可选/不可编辑
    if (shape && !(shape->flags() & QGraphicsItem::ItemIsSelectable))
        shape = nullptr;

    if (shape) {
        // 切换到该图形（直接选择仅编辑顶点，不移动图形本体）
        if (m_activeShape && m_activeShape != shape)
            m_activeShape->setDirectSelected(false);
        scene->clearSelection();
        shape->setSelected(true);
        shape->setDirectSelected(true);
        m_activeShape = shape;

        int idx = anchorIndexAt(event->scenePos(), shape);
        if (idx >= 0) {
            m_anchorDragging = true;
            m_activeAnchor = idx;
            m_origAnchorPoints = shape->anchorPoints();
        }
    } else {
        // 空白处：取消直接选择
        if (m_activeShape)
            m_activeShape->setDirectSelected(false);
        m_activeShape = nullptr;
        scene->clearSelection();
    }
}

void DirectSelectTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (m_anchorDragging && m_activeShape && m_activeAnchor >= 0) {
        m_activeShape->setAnchorPoint(m_activeAnchor, m_activeShape->mapFromScene(event->scenePos()));
        scene->setModified(true);
        return;
    }

    // 悬停检测：鼠标靠近锚点时让该锚点变大
    if (m_activeShape) {
        int idx = anchorIndexAt(event->scenePos(), m_activeShape);
        m_activeShape->setHoveredAnchor(idx);
    }
}

void DirectSelectTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event)

    if (m_anchorDragging && m_activeShape) {
        m_anchorDragging = false;
        if (m_activeShape->anchorPoints() != m_origAnchorPoints)
            scene->pushUndoCommand(new VertexCommand(
                m_activeShape, m_origAnchorPoints, m_activeShape->anchorPoints(), tr("编辑顶点")));
        m_activeAnchor = -1;
        scene->setModified(true);
    }
}
