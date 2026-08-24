#include "CornerRadiusCommand.h"
#include "shapes/RectShape.h"

CornerRadiusCommand::CornerRadiusCommand(RectShape *shape,
                                         qreal oldTL, qreal oldTR, qreal oldBR, qreal oldBL,
                                         qreal newTL, qreal newTR, qreal newBR, qreal newBL,
                                         const QString &text,
                                         QUndoCommand *parent)
    : QUndoCommand(text.isEmpty() ? QObject::tr("调节圆角") : text, parent)
    , m_shape(shape)
    , m_oldTL(oldTL), m_oldTR(oldTR), m_oldBR(oldBR), m_oldBL(oldBL)
    , m_newTL(newTL), m_newTR(newTR), m_newBR(newBR), m_newBL(newBL)
{
}

void CornerRadiusCommand::undo()
{
    if (m_shape)
        m_shape->setCornerRadius(m_oldTL, m_oldTR, m_oldBR, m_oldBL);
}

void CornerRadiusCommand::redo()
{
    if (m_shape)
        m_shape->setCornerRadius(m_newTL, m_newTR, m_newBR, m_newBL);
}
