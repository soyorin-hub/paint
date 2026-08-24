#include "RulerTool.h"
#include "canvas/CanvasScene.h"
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

RulerTool::RulerTool(QObject *parent)
    : ToolBase(parent)
{
}

void RulerTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;
    m_dragging = true;

    RulerGuide r = scene->ruler();
    if (!r.visible) {
        r.visible = true;
        r.orientation = Qt::Horizontal;
    }
    if (r.orientation == Qt::Horizontal)
        r.position = event->scenePos().y();
    else
        r.position = event->scenePos().x();
    scene->setRuler(r);
}

void RulerTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_dragging) return;
    if (scene->ruler().orientation == Qt::Horizontal)
        scene->setRulerPosition(event->scenePos().y());
    else
        scene->setRulerPosition(event->scenePos().x());
}

void RulerTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event) Q_UNUSED(scene)
    m_dragging = false;
}

void RulerTool::keyPressEvent(QKeyEvent *event, CanvasScene *scene)
{
    if (event->key() == Qt::Key_O) {
        scene->toggleRulerOrientation();
        event->accept();
    } else if (event->key() == Qt::Key_H) {
        scene->setRulerVisible(!scene->ruler().visible);
        event->accept();
    }
}
