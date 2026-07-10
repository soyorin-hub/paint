#ifndef TOOLBASE_H
#define TOOLBASE_H

#include <QObject>
#include <QCursor>
#include <QIcon>

class CanvasScene;
class QGraphicsSceneMouseEvent;
class QKeyEvent;

class ToolBase : public QObject
{
    Q_OBJECT

public:
    explicit ToolBase(QObject *parent = nullptr);
    virtual ~ToolBase() = default;

    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) = 0;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) = 0;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) = 0;

    virtual void keyPressEvent(QKeyEvent *event, CanvasScene *scene) { Q_UNUSED(event) Q_UNUSED(scene) }

    virtual QString name() const = 0;
    virtual QIcon icon() const { return QIcon(); }
    virtual QCursor cursor() const { return Qt::CrossCursor; }
    virtual QString shortcut() const { return QString(); }

    virtual void activated() {}
    virtual void deactivated() {}

signals:
    void statusMessage(const QString &msg);
};

#endif // TOOLBASE_H
