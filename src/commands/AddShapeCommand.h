#ifndef ADDSHAPECOMMAND_H
#define ADDSHAPECOMMAND_H

#include <QUndoCommand>
#include <QPointF>

class ShapeBase;
class QGraphicsScene;
class Layer;

class AddShapeCommand : public QUndoCommand
{
public:
    explicit AddShapeCommand(ShapeBase *shape, QGraphicsScene *scene,
                             Layer *layer, const QString &text = QString(),
                             QUndoCommand *parent = nullptr);
    ~AddShapeCommand();

    void undo() override;
    void redo() override;

private:
    ShapeBase *m_shape = nullptr;
    QGraphicsScene *m_scene = nullptr;
    Layer *m_layer = nullptr;
    bool m_ownsShape = true;
};

#endif // ADDSHAPECOMMAND_H
