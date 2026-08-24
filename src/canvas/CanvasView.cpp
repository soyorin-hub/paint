#include "CanvasView.h"
#include "CanvasScene.h"
#include "tools/ToolManager.h"
#include "tools/ToolBase.h"
#include "shapes/ShapeBase.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QPaintEvent>
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
    setFocusPolicy(Qt::ClickFocus);
    setBackgroundBrush(QBrush(QColor(252, 252, 252)));
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void CanvasView::setBackgroundType(int t) { m_bgType = qBound(0, t, 2); viewport()->update(); }
void CanvasView::setRulersVisible(bool visible) { m_showRulers = visible; viewport()->update(); }

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

qreal CanvasView::niceRulerStep(qreal approx)
{
    if (approx < 1e-9) return 1.0;
    qreal e = std::pow(10.0, std::floor(std::log10(approx)));
    qreal f = approx / e;
    qreal nice = (f < 1.5) ? 1.0 : (f < 3.5) ? 2.0 : (f < 7.5) ? 5.0 : 10.0;
    return nice * e;
}

void CanvasView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
    if (!m_showRulers) return;

    QPainter p(viewport());
    p.setRenderHint(QPainter::TextAntialiasing, true);

    // 左上角方块
    p.fillRect(0, 0, RULER_SIZE, RULER_SIZE, QColor(240, 240, 240));
    p.setPen(QColor(180, 180, 180));
    p.drawLine(RULER_SIZE, 0, RULER_SIZE, RULER_SIZE);
    p.drawLine(0, RULER_SIZE, RULER_SIZE, RULER_SIZE);

    drawRuler(&p, Qt::Horizontal, RULER_SIZE);
    drawRuler(&p, Qt::Vertical, RULER_SIZE);
}

void CanvasView::drawRuler(QPainter *p, Qt::Orientation orient, int size)
{
    QRect vr = viewport()->rect();
    QRectF srect = mapToScene(vr).boundingRect();

    qreal approx = 80.0 / m_zoomLevel;   // 主刻度约 80px
    qreal step = niceRulerStep(approx);
    qreal minor = step / 5.0;

    QFont f = p->font();
    f.setPointSizeF(7.0);
    p->setFont(f);
    int dec = (step >= 1.0) ? 0 : 1;

    if (orient == Qt::Horizontal) {
        p->fillRect(0, 0, (int)vr.width(), size, QColor(246, 246, 246));
        qreal k0 = std::floor(srect.left() / minor);
        qreal k1 = std::ceil(srect.right() / minor);
        for (qreal k = k0; k <= k1; k += 1.0) {
            qreal sx = k * minor;
            int vx = mapFromScene(QPointF(sx, 0)).x();
            bool major = (std::llround(k) % 5 == 0);
            int len = major ? 9 : 4;
            p->setPen(QPen(QColor(150, 150, 150), 1));
            p->drawLine(vx, size - len, vx, size);
            if (major) {
                p->setPen(QColor(70, 70, 70));
                p->drawText(QPointF(vx + 3, size - len - 2),
                            QString::number(sx, 'f', dec));
            }
        }
        p->setPen(QColor(180, 180, 180));
        p->drawLine(0, size - 1, (int)vr.width(), size - 1);
    } else {
        p->fillRect(0, 0, size, (int)vr.height(), QColor(246, 246, 246));
        qreal k0 = std::floor(srect.top() / minor);
        qreal k1 = std::ceil(srect.bottom() / minor);
        for (qreal k = k0; k <= k1; k += 1.0) {
            qreal sy = k * minor;
            int vy = mapFromScene(QPointF(0, sy)).y();
            bool major = (std::llround(k) % 5 == 0);
            int len = major ? 9 : 4;
            p->setPen(QPen(QColor(150, 150, 150), 1));
            p->drawLine(size - len, vy, size, vy);
            if (major) {
                p->setPen(QColor(70, 70, 70));
                p->save();
                p->translate(size - len - 2, vy + 3);
                p->rotate(-90);
                p->drawText(QPointF(0, 0), QString::number(sy, 'f', dec));
                p->restore();
            }
        }
        p->setPen(QColor(180, 180, 180));
        p->drawLine(size - 1, 0, size - 1, (int)vr.height());
    }
}

qreal CanvasView::zoomLevel() const { return m_zoomLevel; }
void CanvasView::setZoomLevel(qreal lv) { lv=qBound(MIN_ZOOM,lv,MAX_ZOOM); applyZoom(lv/m_zoomLevel, viewport()->rect().center()); }
void CanvasView::zoomIn()  { applyZoom(1.0+ZOOM_STEP, viewport()->rect().center()); }
void CanvasView::zoomOut() { applyZoom(1.0/(1.0+ZOOM_STEP), viewport()->rect().center()); }
void CanvasView::zoomFit() { if(!scene())return; fitInView(scene()->sceneRect(), Qt::KeepAspectRatio); m_zoomLevel=transform().m11(); emit zoomChanged(m_zoomLevel); }
void CanvasView::applyZoom(qreal f, QPointF c) { qreal nl=m_zoomLevel*f; if(nl<MIN_ZOOM||nl>MAX_ZOOM)return; m_zoomLevel=nl; scale(f,f); emit zoomChanged(m_zoomLevel); }
void CanvasView::wheelEvent(QWheelEvent *e) { if(e->modifiers()&Qt::ControlModifier){ qreal f=(e->angleDelta().y()>0)?(1.0+ZOOM_STEP):1.0/(1.0+ZOOM_STEP); applyZoom(f,e->position()); } else QGraphicsView::wheelEvent(e); }

void CanvasView::keyPressEvent(QKeyEvent *event)
{
    // 文字编辑时，方向键交给编辑器处理
    auto *cs = qobject_cast<CanvasScene*>(scene());
    if (cs && cs->isTextEditing()) {
        QGraphicsView::keyPressEvent(event);
        return;
    }

    qreal step = (event->modifiers() & Qt::ShiftModifier) ? 10.0 : 1.0;
    switch (event->key()) {
    case Qt::Key_Left:  emit nudgeRequested(-step, 0); event->accept(); return;
    case Qt::Key_Right: emit nudgeRequested(step, 0);  event->accept(); return;
    case Qt::Key_Up:    emit nudgeRequested(0, -step); event->accept(); return;
    case Qt::Key_Down:  emit nudgeRequested(0, step);  event->accept(); return;
    default: break;
    }
    // 转发给当前工具（尺子工具用 O/H 切换方向/显示等）
    if (cs && cs->toolManager())
        cs->toolManager()->keyPressEvent(event);
    QGraphicsView::keyPressEvent(event);
}

void CanvasView::contextMenuEvent(QContextMenuEvent *event)
{
    // 无论是否命中图形都发出，由 MainWindow 决定显示哪种菜单
    emit contextMenuRequested(mapToScene(event->pos()), event->globalPos());
}

// ═══════════════════════════════════════════════════
// 鼠标事件
// ═══════════════════════════════════════════════════

static bool toolHandlesAlt(CanvasScene *cs)
{
    if (cs && cs->toolManager() && cs->toolManager()->activeTool())
        return cs->toolManager()->activeTool()->handlesAltModifier();
    return false;
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    // 中键 = 平移
    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true; m_panStart = event->pos();
        setCursor(Qt::ClosedHandCursor); event->accept(); return;
    }

    // Alt+左键 = 平移（仅当工具不自行处理 Alt 时）
    auto *cs = static_cast<CanvasScene*>(scene());
    if (event->button() == Qt::LeftButton
        && (event->modifiers() & Qt::AltModifier)
        && !toolHandlesAlt(cs)) {
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

    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning) { m_isPanning=false; setCursor(Qt::ArrowCursor); event->accept(); return; }

    QGraphicsView::mouseReleaseEvent(event);
}
