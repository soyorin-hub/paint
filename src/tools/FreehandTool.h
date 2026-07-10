#ifndef FREEHANDTOOL_H
#define FREEHANDTOOL_H

#include "ToolBase.h"
#include <QPointF>
#include <QVector>

class FreehandShape;

class FreehandTool : public ToolBase
{
    Q_OBJECT
public:
    explicit FreehandTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void deactivated() override;

    QString name() const override { return tr("画笔"); }
    QIcon icon() const override { return QIcon(":/icons/freehand.svg"); }
    QCursor cursor() const override { return Qt::CrossCursor; }
    QString shortcut() const override { return "P"; }

private:
    QVector<QPointF> m_pts;
    bool m_down = false;
    FreehandShape *m_currentShape = nullptr;
};

#endif
