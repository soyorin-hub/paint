#include "GroupRenameCommand.h"
#include "document/Document.h"

GroupRenameCommand::GroupRenameCommand(Document *document, qint64 groupId,
                                       const QString &oldName, const QString &newName,
                                       QUndoCommand *parent)
    : QUndoCommand(QObject::tr("重命名组"), parent)
    , m_document(document)
    , m_groupId(groupId)
    , m_oldName(oldName)
    , m_newName(newName)
{
}

void GroupRenameCommand::undo()
{
    if (m_document) m_document->setGroupName(m_groupId, m_oldName);
}

void GroupRenameCommand::redo()
{
    if (m_document) m_document->setGroupName(m_groupId, m_newName);
}
