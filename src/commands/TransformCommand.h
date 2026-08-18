#ifndef TRANSFORMCOMMAND_H
#define TRANSFORMCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QTransform>

class QGraphicsItem;

class TransformCommand : public QUndoCommand
{
public:
    TransformCommand(QGraphicsItem *item,
                     const QPointF &oldPos, const QTransform &oldTransform,
                     const QPointF &newPos, const QTransform &newTransform,
                     const QString &text = QString(),
                     QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    QGraphicsItem *m_item = nullptr;
    QPointF m_oldPos, m_newPos;
    QTransform m_oldTransform, m_newTransform;
};

#endif // TRANSFORMCOMMAND_H
