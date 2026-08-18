#include "TextCommand.h"
#include "shapes/TextShape.h"

TextCommand::TextCommand(TextShape *shape,
                         const QString &oldText, const QString &newText,
                         QUndoCommand *parent)
    : QUndoCommand(QObject::tr("编辑文字"), parent)
    , m_shape(shape)
    , m_oldText(oldText)
    , m_newText(newText)
{
}

void TextCommand::undo()
{
    if (m_shape) {
        m_shape->setText(m_oldText);
    }
}

void TextCommand::redo()
{
    if (m_shape) {
        m_shape->setText(m_newText);
    }
}
