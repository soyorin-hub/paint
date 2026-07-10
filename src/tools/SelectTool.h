#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "ToolBase.h"
#include "shapes/ShapeBase.h"

class QGraphicsItem;

class SelectTool : public ToolBase
{
    Q_OBJECT

public:
    explicit SelectTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("选择"); }
    QIcon icon() const override { return QIcon(":/icons/select.svg"); }
    QCursor cursor() const override { return Qt::ArrowCursor; }
    QString shortcut() const override { return "V"; }

private:
    HandleType handleAt(const QPointF &scenePos, ShapeBase *shape) const;
    void applyResize(ShapeBase *shape, HandleType handle, const QPointF &newScenePos);

    bool m_movingItem = false;
    bool m_handleDragging = false;
    HandleType m_activeHandle = Handle_None;
    QPointF m_lastScenePos;
    QPointF m_dragStartPos;
    QGraphicsItem *m_activeItem = nullptr;
    QRectF m_originalRect;   // resize 前的原始矩形
    QPointF m_originalPos;   // resize 前的原始位置
    qreal m_originalRotate = 0;
};

#endif // SELECTTOOL_H
