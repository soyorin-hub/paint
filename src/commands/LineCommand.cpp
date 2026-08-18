#include "LineCommand.h"
#include "shapes/ShapeBase.h"

LineCommand::LineCommand(ShapeBase *shape,
                         const QLineF &oldLine, const QPointF &oldCenter,
                         const QLineF &newLine, const QPointF &newCenter,
                         const QString &text,
                         QUndoCommand *parent)
    : QUndoCommand(text.isEmpty() ? QObject::tr("编辑线段") : text, parent)
    , m_shape(shape)
    , m_oldLine(oldLine)
    , m_newLine(newLine)
    , m_oldCenter(oldCenter)
    , m_newCenter(newCenter)
{
}

void LineCommand::undo()
{
    if (!m_shape) return;
    m_shape->setLinePoint(0, m_oldLine.p1());
    m_shape->setLinePoint(1, m_oldCenter);
    m_shape->setLinePoint(2, m_oldLine.p2());
}

void LineCommand::redo()
{
    if (!m_shape) return;
    m_shape->setLinePoint(0, m_newLine.p1());
    m_shape->setLinePoint(1, m_newCenter);
    m_shape->setLinePoint(2, m_newLine.p2());
}
