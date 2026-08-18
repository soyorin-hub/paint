#include "GroupCommand.h"
#include "shapes/ShapeBase.h"
#include "document/Document.h"

GroupCommand::GroupCommand(Document *document,
                           const QList<ShapeBase*> &shapes,
                           const QList<qint64> &oldIds, const QList<qint64> &newIds,
                           const QMap<qint64, QString> &oldNames, const QMap<qint64, QString> &newNames,
                           const QString &text, QUndoCommand *parent)
    : QUndoCommand(text, parent)
    , m_document(document)
    , m_shapes(shapes)
    , m_oldIds(oldIds)
    , m_newIds(newIds)
    , m_oldNames(oldNames)
    , m_newNames(newNames)
{
}

void GroupCommand::undo()
{
    for (int i = 0; i < m_shapes.size(); ++i)
        if (m_shapes[i]) m_shapes[i]->setGroupId(m_oldIds[i]);
    if (m_document) m_document->setGroupNames(m_oldNames);
}

void GroupCommand::redo()
{
    for (int i = 0; i < m_shapes.size(); ++i)
        if (m_shapes[i]) m_shapes[i]->setGroupId(m_newIds[i]);
    if (m_document) m_document->setGroupNames(m_newNames);
}
