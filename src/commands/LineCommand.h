#ifndef LINECOMMAND_H
#define LINECOMMAND_H

#include <QUndoCommand>
#include <QLineF>
#include <QPointF>

class ShapeBase;

class LineCommand : public QUndoCommand
{
public:
    LineCommand(ShapeBase *shape,
                const QLineF &oldLine, const QPointF &oldCenter,
                const QLineF &newLine, const QPointF &newCenter,
                const QString &text = QString(),
                QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1003; }

private:
    ShapeBase *m_shape = nullptr;
    QLineF m_oldLine, m_newLine;
    QPointF m_oldCenter, m_newCenter;
};

#endif // LINECOMMAND_H
