#ifndef ROTATECOMMAND_H
#define ROTATECOMMAND_H

#include <QUndoCommand>

class ShapeBase;

// 旋转撤销：通过 setRotationAngle 同时还原逻辑角度与变换矩阵
class RotateCommand : public QUndoCommand
{
public:
    RotateCommand(ShapeBase *shape, qreal oldAngle, qreal newAngle,
                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1008; }

private:
    ShapeBase *m_shape = nullptr;
    qreal m_oldAngle = 0.0;
    qreal m_newAngle = 0.0;
};

#endif // ROTATECOMMAND_H
