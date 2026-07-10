#include "CanvasView.h"
#include "CanvasScene.h"
#include "tools/ToolManager.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <cmath>

CanvasView::CanvasView(QWidget *parent) : QGraphicsView(parent)
{
    setRenderHint(QPainter::Antialiasing, true);
    setRenderHint(QPainter::SmoothPixmapTransform, true);
    setDragMode(QGraphicsView::NoDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setMouseTracking(true);
    setBackgroundBrush(QBrush(QColor(252, 252, 252)));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void CanvasView::setBackgroundType(int t) { m_bgType = qBound(0, t, 2); viewport()->update(); }

void CanvasView::drawBackground(QPainter *p, const QRectF &r) {
    switch (m_bgType) {
    case BgWhite: p->fillRect(r, Qt::white); break;
    case BgGrid:  drawGridBackground(p, r);  break;
    default:      drawDotBackground(p, r);   break;
    }
}
void CanvasView::drawDotBackground(QPainter *p, const QRectF &r) {
    p->fillRect(r, QColor(252,252,252)); p->setPen(Qt::NoPen); p->setBrush(QColor(220,220,220));
    qreal s=20, l=std::floor(r.left()/s)*s, t=std::floor(r.top()/s)*s;
    for(qreal x=l; x<r.right(); x+=s) for(qreal y=t; y<r.bottom(); y+=s) p->drawEllipse(QPointF(x,y),1.5,1.5);
}
void CanvasView::drawGridBackground(QPainter *p, const QRectF &r) {
    p->fillRect(r,Qt::white); p->setPen(QPen(QColor(230,230,230),0.5));
    qreal s=20, l=std::floor(r.left()/s)*s, t=std::floor(r.top()/s)*s;
    for(qreal x=l; x<r.right(); x+=s) p->drawLine(QPointF(x,r.top()), QPointF(x,r.bottom()));
    for(qreal y=t; y<r.bottom(); y+=s) p->drawLine(QPointF(r.left(),y), QPointF(r.right(),y));
}

qreal CanvasView::zoomLevel() const { return m_zoomLevel; }
void CanvasView::setZoomLevel(qreal lv) { lv=qBound(MIN_ZOOM,lv,MAX_ZOOM); applyZoom(lv/m_zoomLevel, viewport()->rect().center()); }
void CanvasView::zoomIn()  { applyZoom(1.0+ZOOM_STEP, viewport()->rect().center()); }
void CanvasView::zoomOut() { applyZoom(1.0/(1.0+ZOOM_STEP), viewport()->rect().center()); }
void CanvasView::zoomFit() { if(!scene())return; fitInView(scene()->sceneRect(), Qt::KeepAspectRatio); m_zoomLevel=transform().m11(); emit zoomChanged(m_zoomLevel); }
void CanvasView::applyZoom(qreal f, QPointF c) { qreal nl=m_zoomLevel*f; if(nl<MIN_ZOOM||nl>MAX_ZOOM)return; m_zoomLevel=nl; scale(f,f); emit zoomChanged(m_zoomLevel); }
void CanvasView::wheelEvent(QWheelEvent *e) { if(e->modifiers()&Qt::ControlModifier){ qreal f=(e->angleDelta().y()>0)?(1.0+ZOOM_STEP):1.0/(1.0+ZOOM_STEP); applyZoom(f,e->position()); } else QGraphicsView::wheelEvent(e); }

// ═══════════════════════════════════════════════════
// 核心：CanvasView 始终转发 move 事件给 ToolManager
// ═══════════════════════════════════════════════════

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton ||
        (event->button() == Qt::LeftButton && event->modifiers() & Qt::AltModifier)) {
        m_isPanning = true; m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor); event->accept(); return;
    }
    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPoint d = event->pos() - m_panStart; m_panStart = event->pos();
        horizontalScrollBar()->setValue(horizontalScrollBar()->value()-d.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value()-d.y());
        event->accept(); return;
    }

    // ★ 无条件转发：不管有没有 grabber，都保证 ToolManager 收到 move
    auto *cs = static_cast<CanvasScene*>(scene());
    if (cs && cs->toolManager()) {
        QGraphicsSceneMouseEvent me(QEvent::GraphicsSceneMouseMove);
        me.setWidget(viewport());
        me.setScenePos(mapToScene(event->position().toPoint()));
        me.setScreenPos(event->globalPosition().toPoint());
        me.setButton(Qt::LeftButton);
        me.setButtons(event->buttons());
        me.setModifiers(event->modifiers());
        cs->toolManager()->mouseMoveEvent(&me);
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning) { m_isPanning=false; setCursor(Qt::ArrowCursor); event->accept(); return; }
    QGraphicsView::mouseReleaseEvent(event);
}
