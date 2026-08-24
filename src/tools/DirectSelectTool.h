#ifndef DIRECTSELECTTOOL_H
#define DIRECTSELECTTOOL_H

#include "ToolBase.h"
#include "shapes/ShapeBase.h"
#include <QPointF>
#include <QVector>

class DirectSelectTool : public ToolBase
{
    Q_OBJECT
public:
    explicit DirectSelectTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void deactivated() override;

    QString name() const override { return tr("直接选择"); }
    QIcon icon() const override { return QIcon(":/icons/direct-select.svg"); }
    QCursor cursor() const override { return Qt::ArrowCursor; }
    QString shortcut() const override { return "A"; }

private:
    int anchorIndexAt(const QPointF &scenePos, ShapeBase *shape) const;

    ShapeBase *m_activeShape = nullptr;
    int m_activeAnchor = -1;
    bool m_anchorDragging = false;
    QVector<QPointF> m_origAnchorPoints;
};

#endif // DIRECTSELECTTOOL_H
