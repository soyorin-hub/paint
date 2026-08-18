#ifndef TOOLBASE_H
#define TOOLBASE_H

#include <QObject>
#include <QCursor>
#include <QIcon>
#include <QColor>

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
    virtual void keyReleaseEvent(QKeyEvent *event, CanvasScene *scene) { Q_UNUSED(event) Q_UNUSED(scene) }

    virtual QString name() const = 0;
    virtual QIcon icon() const { return QIcon(); }
    virtual QCursor cursor() const { return Qt::CrossCursor; }
    virtual QString shortcut() const { return QString(); }

    virtual void activated() {}
    virtual void deactivated() {}

    // 是否自行处理 Alt 修饰键（默认 false，由画布用于平移）
    virtual bool handlesAltModifier() const { return false; }

    // 活动图层锁定时是否仍可用（选择工具可用来选中其它图层）
    virtual bool worksOnLockedLayer() const { return false; }

    // ===== 描边样式接口 =====
    // 支持描边设置的工具（图形/画笔）override 这些方法，
    // 使工具栏无需 dynamic_cast 即可统一读写描边参数。
    virtual bool supportsStroke() const { return false; }
    virtual QColor strokeColor() const { return Qt::black; }
    virtual void setStrokeColor(const QColor &) {}
    virtual qreal strokeWidth() const { return 2.0; }
    virtual void setStrokeWidth(qreal) {}
    virtual int strokeStyle() const { return static_cast<int>(Qt::SolidLine); }
    virtual void setStrokeStyle(int) {}

signals:
    void statusMessage(const QString &msg);
};

#endif // TOOLBASE_H
