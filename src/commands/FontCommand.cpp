#include "FontCommand.h"
#include "shapes/TextShape.h"

FontCommand::FontCommand(TextShape *shape,
                         const QFont &oldFont, const QFont &newFont,
                         QUndoCommand *parent)
    : QUndoCommand(QObject::tr("更改字体"), parent)
    , m_shape(shape)
    , m_oldFont(oldFont)
    , m_newFont(newFont)
{
}

void FontCommand::undo()
{
    if (m_shape) {
        m_shape->setFont(m_oldFont);
    }
}

void FontCommand::redo()
{
    if (m_shape) {
        m_shape->setFont(m_newFont);
    }
}
