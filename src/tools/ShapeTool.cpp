#include "ShapeTool.h"
#include "canvas/CanvasScene.h"
#include "commands/AddShapeCommand.h"
#include "shapes/RectShape.h"
#include "shapes/EllipseShape.h"
#include "shapes/LineShape.h"
#include "shapes/TriangleShape.h"
#include "shapes/DiamondShape.h"
#include "shapes/ArrowShape.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

ShapeTool::ShapeTool(QObject *parent)
    : ToolBase(parent)
{
}

bool ShapeTool::isRectBased(ShapeType type) const
{
    return type == Rect || type == Ellipse || type == Triangle || type == Diamond;
}

// ===== 图标生成 =====

QIcon ShapeTool::icon() const
{
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(80, 80, 80), 2.0));
    p.setBrush(Qt::NoBrush);
    p.drawRect(QRectF(2, 3, 9, 8));
    p.drawEllipse(QRectF(13, 3, 9, 8));
    p.drawLine(QLineF(2, 18, 22, 18));
    p.end();
    return QIcon(pix);
}

QIcon ShapeTool::iconForShape(ShapeType type)
{
    QPixmap pix(24, 24);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(60, 60, 60), 1.8));
    p.setBrush(Qt::NoBrush);

    switch (type) {
    case Rect:
        p.drawRect(QRectF(3, 4, 18, 16));
        break;
    case Ellipse:
        p.drawEllipse(QRectF(3, 4, 18, 16));
        break;
    case Line:
        p.drawLine(QLineF(3, 20, 21, 4));
        break;
    case Triangle: {
        QPolygonF tri;
        tri << QPointF(12, 4) << QPointF(21, 20) << QPointF(3, 20);
        p.drawPolygon(tri);
        break;
    }
    case Diamond: {
        QPolygonF dia;
        dia << QPointF(12, 3) << QPointF(21, 12) << QPointF(12, 21) << QPointF(3, 12);
        p.drawPolygon(dia);
        break;
    }
    case Arrow: {
        p.drawLine(QLineF(3, 18, 21, 8));
        qreal dx = 21 - 3, dy = 8 - 18;
        qreal len = std::sqrt(dx*dx + dy*dy);
        dx /= len; dy /= len;
        qreal ax = -dx * 6 - dy * 3.5, ay = -dy * 6 + dx * 3.5;
        qreal bx = -dx * 6 + dy * 3.5, by = -dy * 6 - dx * 3.5;
        p.drawLine(QPointF(21, 8), QPointF(21 + ax, 8 + ay));
        p.drawLine(QPointF(21, 8), QPointF(21 + bx, 8 + by));
        break;
    }
    }
    p.end();
    return QIcon(pix);
}

// ===== Setters =====

void ShapeTool::setShapeType(ShapeType type)
{
    if (m_shapeType != type) {
        m_shapeType = type;
        emit shapeTypeChanged(type);
    }
}

void ShapeTool::setStrokeWidth(qreal width)
{
    if (!qFuzzyCompare(m_strokeWidth, width)) {
        m_strokeWidth = width;
        emit strokeWidthChanged(width);
    }
}

void ShapeTool::setFillColor(const QColor &color)
{
    if (m_fillColor != color) {
        m_fillColor = color;
        emit fillColorChanged(color);
    }
}

void ShapeTool::setStrokeColor(const QColor &color)
{
    if (m_strokeColor != color) {
        m_strokeColor = color;
        emit strokeColorChanged(color);
    }
}

void ShapeTool::setStrokeStyle(int style)
{
    if (m_strokeStyle != style) {
        m_strokeStyle = style;
        emit strokeStyleChanged(style);
    }
}

// ===== 鼠标事件 =====

void ShapeTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_startPoint = event->scenePos();
    m_drawing = true;

    // 记录按下时的 Alt 状态：Windows 上 GetAsyncKeyState 比 event->modifiers() 更可靠
#ifdef Q_OS_WIN
    m_altPressed = GetAsyncKeyState(VK_MENU) & 0x8000;
#else
    m_altPressed = event->modifiers() & Qt::AltModifier;
#endif

    m_realStyle.fillColor   = m_fillColor;
    m_realStyle.strokeColor = m_strokeColor;
    m_realStyle.strokeWidth = m_strokeWidth;
    m_realStyle.penStyle    = static_cast<Qt::PenStyle>(m_strokeStyle);

    m_previewStyle = m_realStyle;
    m_previewStyle.penStyle = Qt::DashLine;

    if (isRectBased(m_shapeType)) {
        switch (m_shapeType) {
        case Rect:     m_currentShape = new RectShape();     break;
        case Ellipse:  m_currentShape = new EllipseShape();  break;
        case Triangle: m_currentShape = new TriangleShape(); break;
        case Diamond:  m_currentShape = new DiamondShape();  break;
        default: return;
        }
        m_currentShape->setPos(m_startPoint);
        m_currentShape->setShapeStyle(m_previewStyle);
        m_currentShape->setOpacity(0.5);

        auto setZeroRect = [](ShapeBase *s) {
            if (auto *r = dynamic_cast<RectShape*>(s)) r->setRect(QRectF(0,0,0,0));
            else if (auto *e = dynamic_cast<EllipseShape*>(s)) e->setRect(QRectF(0,0,0,0));
            else if (auto *t = dynamic_cast<TriangleShape*>(s)) t->setRect(QRectF(0,0,0,0));
            else if (auto *d = dynamic_cast<DiamondShape*>(s)) d->setRect(QRectF(0,0,0,0));
        };
        setZeroRect(m_currentShape);
    } else {
        switch (m_shapeType) {
        case Line:  m_currentShape = new LineShape();  break;
        case Arrow: m_currentShape = new ArrowShape(); break;
        default: return;
        }
        m_currentShape->setPos(0, 0);
        m_currentShape->setShapeStyle(m_previewStyle);
        m_currentShape->setOpacity(0.5);
        QLineF initLine(m_startPoint, m_startPoint);
        if (auto *ls = dynamic_cast<LineShape*>(m_currentShape)) ls->setLine(initLine);
        else if (auto *as = dynamic_cast<ArrowShape*>(m_currentShape)) as->setLine(initLine);
    }

    m_currentShape->setFinished(false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, false);
    scene->addItem(m_currentShape);
}

void ShapeTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(scene)
    if (!m_drawing || !m_currentShape) return;

    QPointF mouse = event->scenePos();
    bool alt   = m_altPressed;
    bool shift = event->modifiers() & Qt::ShiftModifier;

    if (isRectBased(m_shapeType)) {
        QPointF start = m_startPoint;
        qreal w, h;

        if (alt) {
            // 从中心向外绘制：pos 固定在按下点，rect 含负偏移
            qreal hw = qAbs(mouse.x() - start.x());
            qreal hh = qAbs(mouse.y() - start.y());
            if (shift) { hw = hh = qMax(hw, hh); w = h = 2.0 * hw; }
            else       { w = 2.0 * hw; h = 2.0 * hh; }

            m_currentShape->setPos(start);
            QRectF r(-w / 2.0, -h / 2.0, w, h);
            if (auto *rs = dynamic_cast<RectShape*>(m_currentShape)) rs->setRect(r);
            else if (auto *es = dynamic_cast<EllipseShape*>(m_currentShape)) es->setRect(r);
            else if (auto *ts = dynamic_cast<TriangleShape*>(m_currentShape)) ts->setRect(r);
            else if (auto *ds = dynamic_cast<DiamondShape*>(m_currentShape)) ds->setRect(r);
        } else {
            // 从一角向对角
            qreal dx = mouse.x() - start.x();
            qreal dy = mouse.y() - start.y();
            if (shift) {
                qreal size = qMax(qAbs(dx), qAbs(dy));
                w = h = size;
                dx = (dx < 0 ? -size : size);
                dy = (dy < 0 ? -size : size);
            } else {
                w = qAbs(dx); h = qAbs(dy);
            }
            QPointF topLeft(qMin(start.x(), start.x() + dx),
                            qMin(start.y(), start.y() + dy));

            m_currentShape->setPos(topLeft);
            QRectF r(0, 0, w, h);
            if (auto *rs = dynamic_cast<RectShape*>(m_currentShape)) rs->setRect(r);
            else if (auto *es = dynamic_cast<EllipseShape*>(m_currentShape)) es->setRect(r);
            else if (auto *ts = dynamic_cast<TriangleShape*>(m_currentShape)) ts->setRect(r);
            else if (auto *ds = dynamic_cast<DiamondShape*>(m_currentShape)) ds->setRect(r);
        }
    } else {
        // Line / Arrow
        QPointF p1, p2;
        if (alt) {
            QPointF offset = mouse - m_startPoint;
            p1 = m_startPoint - offset;
            p2 = mouse;
        } else {
            p1 = m_startPoint;
            p2 = mouse;
        }
        if (shift) {
            qreal dx = p2.x() - p1.x(), dy = p2.y() - p1.y();
            qreal angle = std::atan2(dy, dx);
            qreal snapped = qRound(angle / (M_PI / 4.0)) * (M_PI / 4.0);
            qreal len = std::sqrt(dx*dx + dy*dy);
            p2 = p1 + QPointF(std::cos(snapped) * len, std::sin(snapped) * len);
        }
        QLineF line(p1, p2);
        if (auto *ls = dynamic_cast<LineShape*>(m_currentShape)) ls->setLine(line);
        else if (auto *as = dynamic_cast<ArrowShape*>(m_currentShape)) as->setLine(line);
    }
}

void ShapeTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_drawing || !m_currentShape) return;
    m_drawing = false;

    QPointF mouse = event->scenePos();
    bool alt   = m_altPressed;
    bool shift = event->modifiers() & Qt::ShiftModifier;

    auto removeShape = [&]() {
        scene->removeItem(m_currentShape);
        delete m_currentShape;
        m_currentShape = nullptr;
    };

    if (isRectBased(m_shapeType)) {
        QPointF start = m_startPoint;
        qreal w, h;

        if (alt) {
            qreal hw = qAbs(mouse.x() - start.x());
            qreal hh = qAbs(mouse.y() - start.y());
            if (shift) { hw = hh = qMax(hw, hh); w = h = 2.0 * hw; }
            else       { w = 2.0 * hw; h = 2.0 * hh; }

            if (w < 3.0 || h < 3.0) { removeShape(); return; }
            m_currentShape->setPos(start.x() - w / 2.0, start.y() - h / 2.0);
        } else {
            qreal dx = mouse.x() - start.x();
            qreal dy = mouse.y() - start.y();
            if (shift) {
                qreal size = qMax(qAbs(dx), qAbs(dy));
                w = h = size;
                dx = (dx < 0 ? -size : size);
                dy = (dy < 0 ? -size : size);
            } else {
                w = qAbs(dx); h = qAbs(dy);
            }
            if (w < 3.0 || h < 3.0) { removeShape(); return; }
            m_currentShape->setPos(qMin(start.x(), start.x() + dx),
                                   qMin(start.y(), start.y() + dy));
        }

        QRectF finalRect(0, 0, w, h);
        if (auto *rs = dynamic_cast<RectShape*>(m_currentShape)) rs->setRect(finalRect);
        else if (auto *es = dynamic_cast<EllipseShape*>(m_currentShape)) es->setRect(finalRect);
        else if (auto *ts = dynamic_cast<TriangleShape*>(m_currentShape)) ts->setRect(finalRect);
        else if (auto *ds = dynamic_cast<DiamondShape*>(m_currentShape)) ds->setRect(finalRect);
    } else {
        QPointF p1, p2;
        if (alt) { QPointF offset = mouse - m_startPoint; p1 = m_startPoint - offset; p2 = mouse; }
        else     { p1 = m_startPoint; p2 = mouse; }
        if (shift) {
            qreal dx = p2.x() - p1.x(), dy = p2.y() - p1.y();
            qreal angle = std::atan2(dy, dx);
            qreal snapped = qRound(angle / (M_PI / 4.0)) * (M_PI / 4.0);
            qreal len = std::sqrt(dx*dx + dy*dy);
            p2 = p1 + QPointF(std::cos(snapped) * len, std::sin(snapped) * len);
        }
        QLineF finalLine(p1, p2);
        if (finalLine.length() < 3.0) { removeShape(); return; }
        if (auto *ls = dynamic_cast<LineShape*>(m_currentShape)) ls->setLine(finalLine);
        else if (auto *as = dynamic_cast<ArrowShape*>(m_currentShape)) as->setLine(finalLine);
    }

    m_currentShape->setFinished(true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_currentShape->setFlag(QGraphicsItem::ItemIsSelectable, true);
    m_currentShape->setShapeStyle(m_realStyle);
    m_currentShape->setOpacity(1.0);

    scene->clearSelection();
    m_currentShape->setSelected(true);

    // 按图形类型生成描述
    QString desc;
    switch (m_shapeType) {
    case Rect:     desc = tr("添加矩形"); break;
    case Ellipse:  desc = tr("添加椭圆"); break;
    case Line:     desc = tr("添加线段"); break;
    case Triangle: desc = tr("添加三角形"); break;
    case Diamond:  desc = tr("添加菱形"); break;
    case Arrow:    desc = tr("添加箭头"); break;
    default:       desc = tr("添加图形"); break;
    }
    scene->pushUndoCommand(new AddShapeCommand(m_currentShape, scene, scene->activeLayer(), desc));
    m_currentShape = nullptr;
    scene->setModified(true);
}
