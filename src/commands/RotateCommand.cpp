#include "RotateCommand.h"
#include "shapes/ShapeBase.h"

RotateCommand::RotateCommand(ShapeBase *shape, qreal oldAngle, qreal newAngle,
                             QUndoCommand *parent)
    : QUndoCommand(QObject::tr("旋转图形"), parent)
    , m_shape(shape)
    , m_oldAngle(oldAngle)
    , m_newAngle(newAngle)
{
}

void RotateCommand::undo()
{
    if (m_shape) {
        m_shape->setRotationAngle(m_oldAngle);
    }
}

void RotateCommand::redo()
{
    if (m_shape) {
        m_shape->setRotationAngle(m_newAngle);
    }
}
