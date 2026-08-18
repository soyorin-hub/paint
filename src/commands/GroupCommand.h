#ifndef GROUPCOMMAND_H
#define GROUPCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QMap>
#include <QString>

class Document;
class ShapeBase;

// 编组/解组撤销：记录每个图形的旧/新 groupId，以及组名表的旧/新快照
class GroupCommand : public QUndoCommand
{
public:
    GroupCommand(Document *document,
                 const QList<ShapeBase*> &shapes,
                 const QList<qint64> &oldIds, const QList<qint64> &newIds,
                 const QMap<qint64, QString> &oldNames, const QMap<qint64, QString> &newNames,
                 const QString &text, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1009; }

private:
    Document *m_document = nullptr;
    QList<ShapeBase*> m_shapes;
    QList<qint64> m_oldIds, m_newIds;
    QMap<qint64, QString> m_oldNames, m_newNames;
};

#endif // GROUPCOMMAND_H
