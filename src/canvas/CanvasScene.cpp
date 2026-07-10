#include "CanvasScene.h"
#include "tools/ToolManager.h"
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

CanvasScene::CanvasScene(QObject *parent) : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 4000, 4000);
    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        auto sel = selectedItems();
        if (sel.isEmpty()) emit itemDeselected();
        else emit itemSelected(sel.first());
    });
}

void CanvasScene::setToolManager(ToolManager *m) { m_toolManager = m; }
void CanvasScene::setModified(bool m) { if (m_modified != m) { m_modified = m; if (m) emit sceneModified(); } }
void CanvasScene::pushUndoCommand(QUndoCommand *cmd) { if (m_toolManager) m_toolManager->pushCommand(cmd); }

void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    if (m_toolManager) m_toolManager->mousePressEvent(event);
}
void CanvasScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if (m_toolManager) m_toolManager->mouseMoveEvent(event);
}
void CanvasScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    if (m_toolManager) m_toolManager->mouseReleaseEvent(event);
}
