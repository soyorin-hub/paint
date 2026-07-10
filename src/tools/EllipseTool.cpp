#include "EllipseTool.h"
#include "shapes/EllipseShape.h"
#include "canvas/CanvasScene.h"
#include "commands/AddShapeCommand.h"
#include <QGraphicsSceneMouseEvent>

EllipseTool::EllipseTool(QObject *parent)
    : ToolBase(parent)
{
}

void EllipseTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_startPoint = event->scenePos();

    m_currentShape = new EllipseShape();
    m_currentShape->setPos(m_startPoint);
    m_currentShape->setRect(QRectF(0, 0, 0, 0));
    m_currentShape->setFinished(false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, false);

    scene->addItem(m_currentShape);
    m_drawing = true;
}

void EllipseTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(scene)
    if (!m_drawing || !m_currentShape) return;

    QPointF localStart = m_currentShape->mapFromScene(m_startPoint);
    QPointF localEnd   = m_currentShape->mapFromScene(event->scenePos());
    m_currentShape->setRect(QRectF(localStart, localEnd).normalized());
}

void EllipseTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_drawing || !m_currentShape) return;

    m_drawing = false;

    QPointF localStart = m_currentShape->mapFromScene(m_startPoint);
    QPointF localEnd   = m_currentShape->mapFromScene(event->scenePos());
    QRectF finalRect = QRectF(localStart, localEnd).normalized();

    if (finalRect.width() < 3 || finalRect.height() < 3) {
        scene->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
        return;
    }

    m_currentShape->setRect(finalRect);
    m_currentShape->setFinished(true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, true);

    scene->clearSelection();
    m_currentShape->setSelected(true);

    scene->pushUndoCommand(new AddShapeCommand(m_currentShape, scene, nullptr));

    m_currentShape = nullptr;
    scene->setModified(true);
}
