#ifndef ELLIPSETOOL_H
#define ELLIPSETOOL_H

#include "ToolBase.h"

class EllipseShape;

class EllipseTool : public ToolBase
{
    Q_OBJECT

public:
    explicit EllipseTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("椭圆"); }
    QIcon icon() const override { return QIcon(":/icons/ellipse.svg"); }
    QString shortcut() const override { return "E"; }

private:
    EllipseShape *m_currentShape = nullptr;
    QPointF m_startPoint;
    bool m_drawing = false;
};

#endif // ELLIPSETOOL_H
