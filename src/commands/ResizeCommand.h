#ifndef RESIZECOMMAND_H
#define RESIZECOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QSizeF>

class ShapeBase;

// 矩形类图形缩放：同时记录位置与尺寸（矩形/椭圆/三角形/菱形）
class ResizeCommand : public QUndoCommand
{
public:
    ResizeCommand(ShapeBase *shape,
                  const QPointF &oldPos, const QSizeF &oldSize,
                  const QPointF &newPos, const QSizeF &newSize,
                  const QString &text = QString(),
                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1005; }

private:
    ShapeBase *m_shape = nullptr;
    QPointF m_oldPos, m_newPos;
    QSizeF m_oldSize, m_newSize;
};

#endif // RESIZECOMMAND_H
