#include "Document.h"
#include "layers/Layer.h"
#include <QJsonArray>
#include <QGraphicsScene>

Document::Document(QObject *parent)
    : QObject(parent)
{
    // 默认创建一个图层
    addLayer(tr("图层 1"));
}

Document::~Document()
{
    qDeleteAll(m_layers);
}

Layer *Document::activeLayer() const
{
    if (m_activeLayerIndex >= 0 && m_activeLayerIndex < m_layers.size())
        return m_layers[m_activeLayerIndex];
    return nullptr;
}

void Document::setActiveLayer(int index)
{
    if (index >= 0 && index < m_layers.size() && index != m_activeLayerIndex) {
        m_activeLayerIndex = index;
        emit activeLayerChanged(m_layers[index]);
    }
}

Layer *Document::addLayer(const QString &name)
{
    Layer *layer = new Layer(name.isEmpty() ? tr("图层 %1").arg(m_layers.size() + 1) : name, this);
    m_layers.append(layer);
    connect(layer, &Layer::changed, this, &Document::modified);
    emit layerAdded(layer, m_layers.size() - 1);
    return layer;
}

void Document::insertLayer(int index, Layer *layer)
{
    if (!layer) return;
    m_layers.insert(index, layer);
    connect(layer, &Layer::changed, this, &Document::modified);
    emit layerAdded(layer, index);
    emit activeLayerChanged(activeLayer());
}

Layer *Document::takeLayer(int index)
{
    if (index < 0 || index >= m_layers.size()) return nullptr;
    Layer *prevActive = activeLayer();
    Layer *layer = m_layers.takeAt(index);
    if (m_activeLayerIndex >= m_layers.size()) {
        m_activeLayerIndex = m_layers.size() - 1;
    }
    emit layerRemoved(index);
    if (activeLayer() != prevActive)
        emit activeLayerChanged(activeLayer());
    return layer;
}

Layer *Document::layerOf(ShapeBase *shape) const
{
    if (!shape) return nullptr;
    for (auto *layer : m_layers) {
        if (layer->shapes().contains(shape))
            return layer;
    }
    return nullptr;
}

void Document::moveLayer(int from, int to)
{
    if (from == to) return;
    if (from < 0 || from >= m_layers.size()) return;
    if (to < 0 || to >= m_layers.size()) return;

    m_layers.move(from, to);
    m_activeLayerIndex = m_layers.indexOf(activeLayer());
    emit layerMoved(from, to);
}

void Document::setCanvasSize(const QSizeF &size)
{
    m_canvasSize = size;
}

void Document::applyToScene(QGraphicsScene *scene)
{
    for (auto *layer : m_layers) {
        layer->applyToScene(scene);
    }
}

// ===== 组名管理 =====

void Document::registerGroup(qint64 id, const QString &name)
{
    if (id < 0 || m_groupNames.contains(id)) return;
    m_groupNames.insert(id, name);
    emit groupsChanged();
}

void Document::unregisterGroup(qint64 id)
{
    if (m_groupNames.remove(id) > 0)
        emit groupsChanged();
}

void Document::setGroupName(qint64 id, const QString &name)
{
    if (!m_groupNames.contains(id) || m_groupNames.value(id) == name) return;
    m_groupNames.insert(id, name);
    emit groupsChanged();
}

QString Document::groupName(qint64 id) const
{
    return m_groupNames.value(id);
}

QList<qint64> Document::groupIds() const
{
    return m_groupNames.keys();
}

void Document::setGroupNames(const QMap<qint64, QString> &names)
{
    if (m_groupNames == names) return;
    m_groupNames = names;
    emit groupsChanged();
}

QJsonObject Document::toJson() const
{
    QJsonObject obj;
    obj["version"] = "1.0";
    obj["canvas"]  = QJsonObject{
        {"width",  m_canvasSize.width()},
        {"height", m_canvasSize.height()}
    };

    QJsonArray layerArray;
    for (auto *layer : m_layers) {
        layerArray.append(layer->toJson());
    }
    obj["layers"] = layerArray;

    QJsonArray groupsArray;
    for (auto it = m_groupNames.constBegin(); it != m_groupNames.constEnd(); ++it) {
        QJsonObject g;
        g["id"]   = static_cast<double>(it.key());
        g["name"] = it.value();
        groupsArray.append(g);
    }
    obj["groups"] = groupsArray;
    return obj;
}

Document *Document::fromJson(const QJsonObject &obj)
{
    Document *doc = new Document();
    doc->clear(); // 清除默认图层

    if (obj.contains("canvas")) {
        QJsonObject c = obj["canvas"].toObject();
        doc->m_canvasSize = QSizeF(c["width"].toDouble(800), c["height"].toDouble(600));
    }

    QJsonArray layerArray = obj["layers"].toArray();
    for (const QJsonValue &val : layerArray) {
        Layer *layer = Layer::fromJson(val.toObject());
        doc->m_layers.append(layer);
        connect(layer, &Layer::changed, doc, &Document::modified);
    }

    if (obj.contains("groups")) {
        QJsonArray groupsArray = obj["groups"].toArray();
        for (const QJsonValue &val : groupsArray) {
            QJsonObject g = val.toObject();
            doc->m_groupNames.insert(
                static_cast<qint64>(g["id"].toDouble()), g["name"].toString());
        }
    }

    if (doc->m_layers.isEmpty()) {
        doc->addLayer(tr("图层 1"));
    }
    return doc;
}

void Document::clear()
{
    qDeleteAll(m_layers);
    m_layers.clear();
    m_activeLayerIndex = 0;
    m_groupNames.clear();
}
