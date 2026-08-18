#ifndef SHAPETOOL_H
#define SHAPETOOL_H

#include "ToolBase.h"
#include "style/ShapeStyle.h"
#include <QColor>

class ShapeBase;

class ShapeTool : public ToolBase
{
    Q_OBJECT

public:
    enum ShapeType {
        Rect, Ellipse, Line,
        Triangle, Diamond, Arrow
    };
    Q_ENUM(ShapeType)

    explicit ShapeTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("图形"); }
    QIcon icon() const override;
    QString shortcut() const override { return "S"; }
    bool handlesAltModifier() const override { return true; }

    bool supportsStroke() const override { return true; }

    ShapeType shapeType() const { return m_shapeType; }
    void setShapeType(ShapeType type);

    qreal strokeWidth() const override { return m_strokeWidth; }
    void setStrokeWidth(qreal width) override;

    QColor fillColor() const { return m_fillColor; }
    void setFillColor(const QColor &color);

    QColor strokeColor() const override { return m_strokeColor; }
    void setStrokeColor(const QColor &color) override;

    int strokeStyle() const override { return m_strokeStyle; }
    void setStrokeStyle(int style) override;

    static QIcon iconForShape(ShapeType type);

signals:
    void shapeTypeChanged(ShapeTool::ShapeType type);
    void strokeWidthChanged(qreal width);
    void fillColorChanged(const QColor &color);
    void strokeColorChanged(const QColor &color);
    void strokeStyleChanged(int style);

private:
    bool isRectBased(ShapeType type) const;

    ShapeType m_shapeType = Rect;
    qreal m_strokeWidth = 2.0;
    QColor m_fillColor = Qt::transparent;
    QColor m_strokeColor = Qt::black;
    int m_strokeStyle = static_cast<int>(Qt::SolidLine);

    ShapeBase *m_currentShape = nullptr;
    QPointF m_startPoint;
    ShapeStyle m_previewStyle;
    ShapeStyle m_realStyle;
    bool m_drawing = false;
    bool m_altPressed = false;
};

#endif // SHAPETOOL_H
