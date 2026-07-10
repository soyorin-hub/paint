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

void Document::removeLayer(int index)
{
    if (index < 0 || index >= m_layers.size()) return;
    if (m_layers.size() <= 1) return; // 至少保留一个图层

    delete m_layers.takeAt(index);
    if (m_activeLayerIndex >= m_layers.size()) {
        m_activeLayerIndex = m_layers.size() - 1;
    }
    emit layerRemoved(index);
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
}
