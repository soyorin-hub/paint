#ifndef CANVASVIEW_H
#define CANVASVIEW_H

#include <QGraphicsView>

class CanvasView : public QGraphicsView
{
    Q_OBJECT
public:
    enum BackgroundType { BgWhite = 0, BgGrid = 1, BgDots = 2 };
    explicit CanvasView(QWidget *parent = nullptr);

    void zoomIn();
    void zoomOut();
    void zoomFit();
    void setZoomLevel(qreal level);
    qreal zoomLevel() const;
    void setBackgroundType(int type);
    int  backgroundType() const { return m_bgType; }
    void setRulersVisible(bool visible);
    bool rulersVisible() const { return m_showRulers; }

signals:
    void zoomChanged(qreal zoomLevel);
    void nudgeRequested(qreal dx, qreal dy);
    void contextMenuRequested(const QPointF &scenePos, const QPoint &globalPos);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void applyZoom(qreal factor, QPointF center);
    void drawDotBackground(QPainter *p, const QRectF &r);
    void drawGridBackground(QPainter *p, const QRectF &r);
    void drawRuler(QPainter *p, Qt::Orientation orient, int size);
    static qreal niceRulerStep(qreal approxSceneUnits);

    qreal  m_zoomLevel = 1.0;
    bool   m_isPanning = false;
    QPoint m_panStart;
    int    m_bgType = BgDots;
    bool   m_showRulers = true;

    static constexpr qreal MIN_ZOOM  = 0.1;
    static constexpr qreal MAX_ZOOM  = 10.0;
    static constexpr qreal ZOOM_STEP = 0.15;
    static constexpr int   RULER_SIZE = 22;
};

#endif
