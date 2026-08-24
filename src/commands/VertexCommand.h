#ifndef VERTEXCOMMAND_H
#define VERTEXCOMMAND_H

#include <QUndoCommand>
#include <QPointF>
#include <QVector>

class ShapeBase;

// 直接选择顶点编辑撤销（保存整条顶点序列）
class VertexCommand : public QUndoCommand
{
public:
    VertexCommand(ShapeBase *shape,
                  const QVector<QPointF> &oldPoints,
                  const QVector<QPointF> &newPoints,
                  const QString &text = QString(),
                  QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    int id() const override { return 1014; }

private:
    ShapeBase *m_shape = nullptr;
    QVector<QPointF> m_oldPoints;
    QVector<QPointF> m_newPoints;
};

#endif // VERTEXCOMMAND_H
