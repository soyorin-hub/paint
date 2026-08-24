#ifndef FREEHANDTOOL_H
#define FREEHANDTOOL_H

#include "ToolBase.h"
#include "style/ShapeStyle.h"
#include <QPointF>
#include <QVector>
#include <QColor>

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

    QString name() const override { return tr("自由钢笔"); }
    QIcon icon() const override { return QIcon(":/icons/freehand.svg"); }
    QCursor cursor() const override { return Qt::CrossCursor; }
    QString shortcut() const override { return "P"; }

    bool supportsStroke() const override { return true; }

    QColor strokeColor() const override { return m_strokeColor; }
    void setStrokeColor(const QColor &color) override;

    qreal strokeWidth() const override { return m_strokeWidth; }
    void setStrokeWidth(qreal width) override;

    int strokeStyle() const override { return m_strokeStyle; }
    void setStrokeStyle(int style) override;

signals:
    void strokeColorChanged(const QColor &color);
    void strokeWidthChanged(qreal width);
    void strokeStyleChanged(int style);

private:
    QVector<QPointF> m_pts;
    bool m_down = false;
    FreehandShape *m_currentShape = nullptr;
    ShapeStyle m_realStyle;

    QColor m_strokeColor{200, 30, 30};
    qreal m_strokeWidth = 3.0;
    int m_strokeStyle = static_cast<int>(Qt::SolidLine);
};

#endif
