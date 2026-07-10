#ifndef REMOVESHAPECOMMAND_H
#define REMOVESHAPECOMMAND_H

#include <QUndoCommand>

class ShapeBase;
class QGraphicsScene;
class Layer;

class RemoveShapeCommand : public QUndoCommand
{
public:
    explicit RemoveShapeCommand(ShapeBase *shape, QGraphicsScene *scene,
                                Layer *layer, QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

private:
    ShapeBase *m_shape = nullptr;
    QGraphicsScene *m_scene = nullptr;
    Layer *m_layer = nullptr;
    bool m_removed = false;
};

#endif // REMOVESHAPECOMMAND_H
