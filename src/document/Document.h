#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QSizeF>
#include <QJsonObject>

class Layer;
class QGraphicsScene;

class Document : public QObject
{
    Q_OBJECT

public:
    explicit Document(QObject *parent = nullptr);
    ~Document();

    // 图层管理
    Layer *activeLayer() const;
    int activeLayerIndex() const { return m_activeLayerIndex; }
    void setActiveLayer(int index);
    Layer *addLayer(const QString &name = QString());
    void removeLayer(int index);
    void moveLayer(int from, int to);
    QList<Layer*> layers() const { return m_layers; }
    int layerCount() const { return m_layers.size(); }

    // 画布尺寸
    QSizeF canvasSize() const { return m_canvasSize; }
    void setCanvasSize(const QSizeF &size);

    // 将全部图层应用到场景
    void applyToScene(QGraphicsScene *scene);

    // 序列化
    QJsonObject toJson() const;
    static Document *fromJson(const QJsonObject &obj);

    // 清除
    void clear();

signals:
    void layerAdded(Layer *layer, int index);
    void layerRemoved(int index);
    void layerMoved(int from, int to);
    void activeLayerChanged(Layer *layer);
    void modified();

private:
    QList<Layer*> m_layers;
    int m_activeLayerIndex = 0;
    QSizeF m_canvasSize{800, 600};
};

#endif // DOCUMENT_H
