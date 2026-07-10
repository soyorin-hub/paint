#include "RemoveShapeCommand.h"
#include "shapes/ShapeBase.h"
#include "layers/Layer.h"
#include <QGraphicsScene>

RemoveShapeCommand::RemoveShapeCommand(ShapeBase *shape, QGraphicsScene *scene,
                                         Layer *layer, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("删除图形"), parent)
    , m_shape(shape)
    , m_scene(scene)
    , m_layer(layer)
{
}

void RemoveShapeCommand::undo()
{
    if (m_shape && m_scene && m_removed) {
        m_scene->addItem(m_shape);
        if (m_layer) m_layer->addShape(m_shape);
        m_removed = false;
    }
}

void RemoveShapeCommand::redo()
{
    if (m_shape && m_scene && !m_removed) {
        m_scene->removeItem(m_shape);
        if (m_layer) m_layer->removeShape(m_shape);
        m_removed = true;
    }
}
