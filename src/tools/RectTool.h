#ifndef RECTTOOL_H
#define RECTTOOL_H

#include "ToolBase.h"

class RectShape;

class RectTool : public ToolBase
{
    Q_OBJECT

public:
    explicit RectTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("矩形"); }
    QIcon icon() const override { return QIcon(":/icons/rect.svg"); }
    QString shortcut() const override { return "R"; }

private:
    RectShape *m_currentShape = nullptr;
    QPointF m_startPoint;
    bool m_drawing = false;
};

#endif // RECTTOOL_H
