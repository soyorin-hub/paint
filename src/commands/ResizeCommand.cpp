#include "ResizeCommand.h"
#include "shapes/ShapeBase.h"

ResizeCommand::ResizeCommand(ShapeBase *shape,
                             const QPointF &oldPos, const QSizeF &oldSize,
                             const QPointF &newPos, const QSizeF &newSize,
                             const QString &text,
                             QUndoCommand *parent)
    : QUndoCommand(text.isEmpty() ? QObject::tr("缩放图形") : text, parent)
    , m_shape(shape)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
    , m_oldSize(oldSize)
    , m_newSize(newSize)
{
}

void ResizeCommand::undo()
{
    if (m_shape) {
        m_shape->setPos(m_oldPos);
        m_shape->setSize(m_oldSize);
    }
}

void ResizeCommand::redo()
{
    if (m_shape) {
        m_shape->setPos(m_newPos);
        m_shape->setSize(m_newSize);
    }
}
