#include "LineTool.h"
#include "shapes/LineShape.h"
#include "canvas/CanvasScene.h"
#include "commands/AddShapeCommand.h"
#include <QGraphicsSceneMouseEvent>

LineTool::LineTool(QObject *parent)
    : ToolBase(parent)
{
}

void LineTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_startPoint = event->scenePos();

    m_currentShape = new LineShape();
    m_currentShape->setPos(0, 0);  // 线段使用绝对坐标
    m_currentShape->setLine(QLineF(m_startPoint, m_startPoint));
    m_currentShape->setFinished(false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, false);

    scene->addItem(m_currentShape);
    m_drawing = true;
}

void LineTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(scene)
    if (!m_drawing || !m_currentShape) return;

    m_currentShape->setLine(QLineF(m_startPoint, event->scenePos()));
}

void LineTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_drawing || !m_currentShape) return;

    m_drawing = false;

    QLineF finalLine(m_startPoint, event->scenePos());
    if (finalLine.length() < 3.0) {
        scene->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
        return;
    }

    m_currentShape->setLine(finalLine);
    m_currentShape->setFinished(true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, true);

    scene->clearSelection();
    m_currentShape->setSelected(true);

    scene->pushUndoCommand(new AddShapeCommand(m_currentShape, scene, nullptr));

    m_currentShape = nullptr;
    scene->setModified(true);
}
