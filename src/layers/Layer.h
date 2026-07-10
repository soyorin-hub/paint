#ifndef LAYER_H
#define LAYER_H

#include <QObject>
#include <QList>
#include <QString>
#include <QJsonObject>

class ShapeBase;
class QGraphicsScene;

class Layer : public QObject
{
    Q_OBJECT

public:
    explicit Layer(const QString &name = QString(), QObject *parent = nullptr);
    ~Layer();

    // 名称
    QString name() const { return m_name; }
    void setName(const QString &name);

    // 可见性
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);

    // 锁定
    bool isLocked() const { return m_locked; }
    void setLocked(bool locked);

    // 透明度
    qreal opacity() const { return m_opacity; }
    void setOpacity(qreal opacity);

    // 图形管理
    void addShape(ShapeBase *shape);
    void removeShape(ShapeBase *shape);
    QList<ShapeBase*> shapes() const { return m_shapes; }
    int shapeCount() const { return m_shapes.size(); }
    bool isEmpty() const { return m_shapes.isEmpty(); }

    // 应用到场景
    void applyToScene(QGraphicsScene *scene);
    void removeFromScene(QGraphicsScene *scene);

    // 序列化
    QJsonObject toJson() const;
    static Layer *fromJson(const QJsonObject &obj);

signals:
    void changed();
    void nameChanged(const QString &name);
    void visibilityChanged(bool visible);
    void lockChanged(bool locked);

private:
    QString m_name;
    bool m_visible = true;
    bool m_locked = false;
    qreal m_opacity = 1.0;
    QList<ShapeBase*> m_shapes;
};

#endif // LAYER_H
