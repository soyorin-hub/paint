#ifndef LAYERCOMMAND_H
#define LAYERCOMMAND_H

#include <QUndoCommand>
#include <QString>

class Document;
class Layer;
class QGraphicsScene;

// 图层操作撤销：新增 / 删除 / 移动 / 重命名
class LayerCommand : public QUndoCommand
{
public:
    enum Operation { AddLayer, RemoveLayer, MoveLayer, RenameLayer };

    LayerCommand(Operation op, Document *document, QGraphicsScene *scene,
                 Layer *layer, int index, int targetIndex, const QString &newName,
                 QUndoCommand *parent = nullptr);
    ~LayerCommand() override;

    void undo() override;
    void redo() override;

    int id() const override { return 1007; }

private:
    Operation m_op;
    Document *m_document = nullptr;
    QGraphicsScene *m_scene = nullptr;
    Layer *m_layer = nullptr;      // Add/Remove 的图层；Rename 的目标层
    int m_index = -1;              // Add/Remove 的索引；Move 的 from
    int m_targetIndex = -1;        // Move 的 to
    QString m_oldName, m_newName;  // Rename
    bool m_ownsLayer = false;      // 图层当前不在 document 中（由本命令持有并负责释放）
};

#endif // LAYERCOMMAND_H
