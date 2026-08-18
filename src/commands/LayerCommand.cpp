#include "LayerCommand.h"
#include "document/Document.h"
#include "layers/Layer.h"
#include <QGraphicsScene>

LayerCommand::LayerCommand(Operation op, Document *document, QGraphicsScene *scene,
                           Layer *layer, int index, int targetIndex, const QString &newName,
                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_op(op)
    , m_document(document)
    , m_scene(scene)
    , m_layer(layer)
    , m_index(index)
    , m_targetIndex(targetIndex)
    , m_newName(newName)
{
    switch (op) {
    case AddLayer:    setText(QObject::tr("添加图层")); break;
    case RemoveLayer: setText(QObject::tr("删除图层")); break;
    case MoveLayer:   setText(QObject::tr("移动图层")); break;
    case RenameLayer: setText(QObject::tr("重命名图层"));
                      m_oldName = m_layer ? m_layer->name() : QString();
                      break;
    }
}

LayerCommand::~LayerCommand()
{
    // 图层已移出 document 时由本命令负责释放（其图形已在删除时从场景移除）
    if (m_ownsLayer)
        delete m_layer;
}

void LayerCommand::redo()
{
    if (!m_document) return;
    switch (m_op) {
    case AddLayer:
        if (m_layer) {
            // 撤销后重做：重新插入原图层
            m_document->insertLayer(m_index, m_layer);
        } else {
            // 首次执行：新建图层
            m_layer = m_document->addLayer();
        }
        m_ownsLayer = false;
        break;
    case RemoveLayer:
        m_layer = m_document->takeLayer(m_index);
        if (m_layer && m_scene) m_layer->removeFromScene(m_scene);
        m_ownsLayer = true;
        break;
    case MoveLayer:
        m_document->moveLayer(m_index, m_targetIndex);
        break;
    case RenameLayer:
        if (m_layer) m_layer->setName(m_newName);
        break;
    }
}

void LayerCommand::undo()
{
    if (!m_document) return;
    switch (m_op) {
    case AddLayer:
        if (m_layer) {
            m_layer = m_document->takeLayer(m_index);
            m_ownsLayer = true;
        }
        break;
    case RemoveLayer:
        if (m_layer) {
            m_document->insertLayer(m_index, m_layer);
            if (m_scene) m_layer->applyToScene(m_scene);
            m_ownsLayer = false;
        }
        break;
    case MoveLayer:
        m_document->moveLayer(m_targetIndex, m_index);
        break;
    case RenameLayer:
        if (m_layer) m_layer->setName(m_oldName);
        break;
    }
}
