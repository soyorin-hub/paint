#ifndef ERASERTOOL_H
#define ERASERTOOL_H

#include "ToolBase.h"
#include <QRectF>

class QGraphicsRectItem;

class EraserTool : public ToolBase
{
    Q_OBJECT

public:
    explicit EraserTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("橡皮擦"); }
    QIcon icon() const override;
    QCursor cursor() const override { return Qt::ForbiddenCursor; }
    QString shortcut() const override { return "X"; }

private:
    QPointF m_startPoint;
    QGraphicsRectItem *m_previewRect = nullptr;
    bool m_erasing = false;
};

#endif // ERASERTOOL_H
