#include "TextTool.h"
#include "shapes/TextShape.h"
#include "canvas/CanvasScene.h"
#include "commands/AddShapeCommand.h"
#include <QGraphicsSceneMouseEvent>

TextTool::TextTool(QObject *parent)
    : ToolBase(parent)
{
    m_defaultFont = QFont("Microsoft YaHei", 16);
}

QIcon TextTool::icon() const
{
    return QIcon(":/icons/text.svg");
}

void TextTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    TextShape *shape = new TextShape();
    shape->setPos(event->scenePos());
    shape->setFont(m_defaultFont);
    shape->setFinished(true);

    scene->addItem(shape);
    scene->clearSelection();
    shape->setSelected(true);
    scene->pushUndoCommand(new AddShapeCommand(shape, scene, scene->activeLayer(),
                                                tr("添加文字")));
    scene->setModified(true);
}

void TextTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event) Q_UNUSED(scene)
}

void TextTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(event) Q_UNUSED(scene)
}
