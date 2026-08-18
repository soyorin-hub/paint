#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QSizeF>
#include <QJsonObject>

class Layer;
class QGraphicsScene;
class ShapeBase;

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
    void insertLayer(int index, Layer *layer);
    Layer *takeLayer(int index);
    void moveLayer(int from, int to);
    QList<Layer*> layers() const { return m_layers; }
    int layerCount() const { return m_layers.size(); }
    Layer *layerOf(ShapeBase *shape) const;

    // 画布尺寸
    QSizeF canvasSize() const { return m_canvasSize; }
    void setCanvasSize(const QSizeF &size);

    // 组名管理
    void registerGroup(qint64 id, const QString &name);
    void unregisterGroup(qint64 id);
    void setGroupName(qint64 id, const QString &name);
    QString groupName(qint64 id) const;
    QList<qint64> groupIds() const;
    QMap<qint64, QString> groupNames() const { return m_groupNames; }
    void setGroupNames(const QMap<qint64, QString> &names);

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
    void groupsChanged();

private:
    QList<Layer*> m_layers;
    int m_activeLayerIndex = 0;
    QSizeF m_canvasSize{800, 600};
    QMap<qint64, QString> m_groupNames;
};

#endif // DOCUMENT_H
