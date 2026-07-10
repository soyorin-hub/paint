#ifndef LINETOOL_H
#define LINETOOL_H

#include "ToolBase.h"

class LineShape;

class LineTool : public ToolBase
{
    Q_OBJECT

public:
    explicit LineTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("线段"); }
    QIcon icon() const override { return QIcon(":/icons/line.svg"); }
    QString shortcut() const override { return "L"; }

private:
    LineShape *m_currentShape = nullptr;
    QPointF m_startPoint;
    bool m_drawing = false;
};

#endif // LINETOOL_H
