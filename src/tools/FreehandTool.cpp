#include "FreehandTool.h"
#include "shapes/FreehandShape.h"
#include "canvas/CanvasScene.h"
#include "commands/AddShapeCommand.h"
#include <QGraphicsSceneMouseEvent>
#include <QLineF>

FreehandTool::FreehandTool(QObject *parent) : ToolBase(parent) {}

void FreehandTool::deactivated()
{
    m_down = false;
    m_pts.clear();
    // 如果切换工具时还有未完成的形状，清理掉
    if (m_currentShape) {
        if (m_currentShape->scene())
            m_currentShape->scene()->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
    }
}

void FreehandTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_pts.clear();
    QPointF pt = event->scenePos();
    m_pts.append(pt);

    // 创建预览形状（与 LineTool/RectTool 模式一致）
    m_currentShape = new FreehandShape();
    m_currentShape->setPos(0, 0);
    ShapeStyle s;
    s.fillColor   = Qt::transparent;
    s.strokeColor = QColor(200, 30, 30);
    s.strokeWidth = 3.0;
    m_currentShape->setShapeStyle(s);
    m_currentShape->addPoint(pt);
    m_currentShape->setFinished(false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, false);

    scene->addItem(m_currentShape);
    m_down = true;
}

void FreehandTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(scene)
    if (!m_down || !m_currentShape) return;

    QPointF pt = event->scenePos();
    // 过滤过近的点，避免路径过于密集
    if (!m_pts.isEmpty() && QLineF(m_pts.last(), pt).length() < 3.0) return;

    m_pts.append(pt);
    m_currentShape->addPoint(pt);
}

void FreehandTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_down || !m_currentShape) return;
    m_down = false;

    m_pts.append(event->scenePos());
    m_currentShape->addPoint(event->scenePos());

    // 去重+过滤：路径太短则丢弃
    if (m_pts.size() < 2) {
        scene->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
        m_pts.clear();
        return;
    }
    qreal len = 0;
    for (int i = 1; i < m_pts.size(); ++i) len += QLineF(m_pts[i-1], m_pts[i]).length();
    if (len < 5.0) {
        scene->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
        m_pts.clear();
        return;
    }

    m_currentShape->setFinished(true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, true);

    scene->clearSelection();
    m_currentShape->setSelected(true);

    scene->pushUndoCommand(new AddShapeCommand(m_currentShape, scene, nullptr));
    scene->setModified(true);

    m_currentShape = nullptr;
    m_pts.clear();
}
