#include "StyleCommand.h"
#include "shapes/ShapeBase.h"

StyleCommand::StyleCommand(ShapeBase *shape,
                             const ShapeStyle &oldStyle, const ShapeStyle &newStyle,
                             QUndoCommand *parent)
    : QUndoCommand(QObject::tr("更改样式"), parent)
    , m_shape(shape)
    , m_oldStyle(oldStyle)
    , m_newStyle(newStyle)
{
}

void StyleCommand::undo()
{
    if (m_shape) {
        m_shape->setShapeStyle(m_oldStyle);
    }
}

void StyleCommand::redo()
{
    if (m_shape) {
        m_shape->setShapeStyle(m_newStyle);
    }
}

bool StyleCommand::mergeWith(const QUndoCommand *other)
{
    const StyleCommand *cmd = dynamic_cast<const StyleCommand*>(other);
    if (!cmd || cmd->m_shape != m_shape) return false;

    m_newStyle = cmd->m_newStyle;
    return true;
}
