#include "ReorderShapesCommand.h"
#include "layers/Layer.h"

ReorderShapesCommand::ReorderShapesCommand(const QList<Layer*> &layers,
                                           const QList<QList<ShapeBase*>> &oldOrders,
                                           const QList<QList<ShapeBase*>> &newOrders,
                                           const QString &text, QUndoCommand *parent)
    : QUndoCommand(text, parent)
    , m_layers(layers)
    , m_oldOrders(oldOrders)
    , m_newOrders(newOrders)
{
}

void ReorderShapesCommand::undo()
{
    for (int i = 0; i < m_layers.size(); ++i)
        if (m_layers[i]) m_layers[i]->reorderShapes(m_oldOrders[i]);
}

void ReorderShapesCommand::redo()
{
    for (int i = 0; i < m_layers.size(); ++i)
        if (m_layers[i]) m_layers[i]->reorderShapes(m_newOrders[i]);
}
