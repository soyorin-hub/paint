#include "TransformCommand.h"
#include <QGraphicsItem>

TransformCommand::TransformCommand(QGraphicsItem *item,
                                     const QPointF &oldPos, const QTransform &oldTransform,
                                     const QPointF &newPos, const QTransform &newTransform,
                                     const QString &text,
                                     QUndoCommand *parent)
    : QUndoCommand(text.isEmpty() ? QObject::tr("移动/变换") : text, parent)
    , m_item(item)
    , m_oldPos(oldPos), m_newPos(newPos)
    , m_oldTransform(oldTransform), m_newTransform(newTransform)
{
}

void TransformCommand::undo()
{
    if (m_item) {
        m_item->setPos(m_oldPos);
        m_item->setTransform(m_oldTransform);
    }
}

void TransformCommand::redo()
{
    if (m_item) {
        m_item->setPos(m_newPos);
        m_item->setTransform(m_newTransform);
    }
}

bool TransformCommand::mergeWith(const QUndoCommand *other)
{
    const TransformCommand *cmd = dynamic_cast<const TransformCommand*>(other);
    if (!cmd || cmd->m_item != m_item) return false;

    // 合并连续的移动命令
    m_newPos = cmd->m_newPos;
    m_newTransform = cmd->m_newTransform;
    return true;
}
