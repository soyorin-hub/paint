#include "AddShapeCommand.h"
#include "shapes/ShapeBase.h"
#include "layers/Layer.h"
#include <QGraphicsScene>

AddShapeCommand::AddShapeCommand(ShapeBase *shape, QGraphicsScene *scene,
                                   Layer *layer, QUndoCommand *parent)
    : QUndoCommand(QObject::tr("添加图形"), parent)
    , m_shape(shape)
    , m_scene(scene)
    , m_layer(layer)
{
}

AddShapeCommand::~AddShapeCommand()
{
    if (m_ownsShape) {
        // shape 已被场景移除时由 undo 负责删除
    }
}

void AddShapeCommand::undo()
{
    if (m_shape && m_scene) {
        m_scene->removeItem(m_shape);
        if (m_layer) m_layer->removeShape(m_shape);
        m_ownsShape = true;
    }
}

void AddShapeCommand::redo()
{
    if (m_shape && m_scene) {
        m_scene->addItem(m_shape);
        if (m_layer) m_layer->addShape(m_shape);
        m_ownsShape = false;
    }
}
