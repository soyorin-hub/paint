#ifndef REORDERSHAPESCOMMAND_H
#define REORDERSHAPESCOMMAND_H

#include <QUndoCommand>
#include <QList>
#include <QString>

class Layer;
class ShapeBase;

// 图层内图形重排（置顶/置底/上移/下移）的撤销
class ReorderShapesCommand : public QUndoCommand
{
public:
    ReorderShapesCommand(const QList<Layer*> &layers,
                         const QList<QList<ShapeBase*>> &oldOrders,
                         const QList<QList<ShapeBase*>> &newOrders,
                         const QString &text, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1010; }

private:
    QList<Layer*> m_layers;
    QList<QList<ShapeBase*>> m_oldOrders, m_newOrders;
};

#endif // REORDERSHAPESCOMMAND_H
