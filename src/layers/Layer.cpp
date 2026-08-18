#include "Layer.h"
#include "shapes/ShapeBase.h"
#include <QGraphicsScene>
#include <QJsonArray>

Layer::Layer(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name.isEmpty() ? tr("图层 %1").arg(1) : name)
{
}

Layer::~Layer()
{
    qDeleteAll(m_shapes);
}

void Layer::setName(const QString &name)
{
    if (m_name != name) {
        m_name = name;
        emit nameChanged(m_name);
        emit changed();
    }
}

void Layer::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (auto *shape : m_shapes) {
            shape->setVisible(visible);
        }
        emit visibilityChanged(m_visible);
        emit changed();
    }
}

void Layer::setLocked(bool locked)
{
    if (m_locked != locked) {
        m_locked = locked;
        // 锁定图层上的图形不可选中/移动
        for (auto *shape : m_shapes) {
            shape->setFlag(QGraphicsItem::ItemIsSelectable, !locked);
            shape->setFlag(QGraphicsItem::ItemIsMovable, !locked);
        }
        emit lockChanged(m_locked);
        emit changed();
    }
}

void Layer::setOpacity(qreal opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    for (auto *shape : m_shapes) {
        shape->setOpacity(m_opacity);
    }
    emit changed();
}

void Layer::addShape(ShapeBase *shape)
{
    if (!shape || m_shapes.contains(shape)) return;
    m_shapes.append(shape);
    shape->setVisible(m_visible);
    shape->setOpacity(m_opacity);
    shape->setFlag(QGraphicsItem::ItemIsSelectable, !m_locked);
    shape->setFlag(QGraphicsItem::ItemIsMovable, !m_locked);
    emit changed();
}

void Layer::removeShape(ShapeBase *shape)
{
    if (m_shapes.removeOne(shape)) {
        emit changed();
    }
}

void Layer::reorderShapes(const QList<ShapeBase*> &newOrder)
{
    if (newOrder.size() != m_shapes.size()) return;
    m_shapes = newOrder;
    emit changed();
}

void Layer::applyToScene(QGraphicsScene *scene)
{
    for (auto *shape : m_shapes) {
        if (shape->scene() != scene) {
            scene->addItem(shape);
        }
    }
}

void Layer::removeFromScene(QGraphicsScene *scene)
{
    for (auto *shape : m_shapes) {
        if (shape->scene() == scene) {
            scene->removeItem(shape);
        }
    }
}

QJsonObject Layer::toJson() const
{
    QJsonObject obj;
    obj["name"]    = m_name;
    obj["visible"] = m_visible;
    obj["locked"]  = m_locked;
    obj["opacity"] = m_opacity;

    QJsonArray shapeArray;
    for (auto *shape : m_shapes) {
        shapeArray.append(shape->toJson());
    }
    obj["shapes"] = shapeArray;
    return obj;
}

Layer *Layer::fromJson(const QJsonObject &obj)
{
    Layer *layer = new Layer(obj["name"].toString());
    layer->m_visible = obj["visible"].toBool(true);
    layer->m_locked  = obj["locked"].toBool(false);
    layer->m_opacity = obj["opacity"].toDouble(1.0);

    QJsonArray shapeArray = obj["shapes"].toArray();
    for (const QJsonValue &val : shapeArray) {
        ShapeBase *shape = ShapeBase::createFromJson(val.toObject());
        if (shape) {
            layer->m_shapes.append(shape);
        }
    }
    return layer;
}
