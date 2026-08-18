#ifndef GROUPRENAMECOMMAND_H
#define GROUPRENAMECOMMAND_H

#include <QUndoCommand>
#include <QString>

class Document;

// 组重命名撤销
class GroupRenameCommand : public QUndoCommand
{
public:
    GroupRenameCommand(Document *document, qint64 groupId,
                       const QString &oldName, const QString &newName,
                       QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1011; }

private:
    Document *m_document = nullptr;
    qint64 m_groupId = -1;
    QString m_oldName, m_newName;
};

#endif // GROUPRENAMECOMMAND_H
