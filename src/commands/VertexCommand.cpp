#include "VertexCommand.h"
#include "shapes/ShapeBase.h"

VertexCommand::VertexCommand(ShapeBase *shape,
                             const QVector<QPointF> &oldPoints,
                             const QVector<QPointF> &newPoints,
                             const QString &text,
                             QUndoCommand *parent)
    : QUndoCommand(text.isEmpty() ? QObject::tr("编辑顶点") : text, parent)
    , m_shape(shape)
    , m_oldPoints(oldPoints)
    , m_newPoints(newPoints)
{
}

void VertexCommand::undo()
{
    if (m_shape)
        m_shape->setAnchorPoints(m_oldPoints);
}

void VertexCommand::redo()
{
    if (m_shape)
        m_shape->setAnchorPoints(m_newPoints);
}
