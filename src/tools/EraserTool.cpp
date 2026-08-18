#include "EraserTool.h"
#include "canvas/CanvasScene.h"
#include "shapes/ShapeBase.h"
#include "commands/RemoveShapeCommand.h"
#include "tools/ToolManager.h"
#include "document/Document.h"
#include "layers/Layer.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsRectItem>
#include <QPen>

EraserTool::EraserTool(QObject *parent)
    : ToolBase(parent)
{
}

QIcon EraserTool::icon() const
{
    return QIcon(":/icons/eraser.svg");
}

void EraserTool::mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (event->button() != Qt::LeftButton) return;

    m_startPoint = event->scenePos();

    // 创建虚线预览矩形
    m_previewRect = new QGraphicsRectItem();
    m_previewRect->setPen(QPen(QColor(255, 80, 80), 1.5, Qt::DashLine));
    m_previewRect->setBrush(QColor(255, 0, 0, 30));
    m_previewRect->setZValue(10000);
    scene->addItem(m_previewRect);
    m_erasing = true;
}

void EraserTool::mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    Q_UNUSED(scene)
    if (!m_erasing || !m_previewRect) return;

    QRectF rect = QRectF(m_startPoint, event->scenePos()).normalized();
    m_previewRect->setRect(rect);
}

void EraserTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene)
{
    if (!m_erasing) return;

    m_erasing = false;

    // 删除预览矩形
    if (m_previewRect) {
        scene->removeItem(m_previewRect);
        delete m_previewRect;
        m_previewRect = nullptr;
    }

    // 获取橡皮擦区域
    QRectF eraseRect = QRectF(m_startPoint, event->scenePos()).normalized();
    if (eraseRect.width() < 3 || eraseRect.height() < 3) return;

    // 收集框内/相交的所有图形
    QList<QGraphicsItem*> items = scene->items(eraseRect, Qt::IntersectsItemShape);
    if (items.isEmpty()) return;

    // 批量删除
    auto *manager = scene->toolManager();
    Document *doc = scene->document();
    if (manager && manager->undoStack()) {
        manager->undoStack()->beginMacro(tr("橡皮擦删除"));
        for (auto *item : items) {
            ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
            if (shape && !shape->parentItem()) {
                Layer *layer = doc ? doc->layerOf(shape) : nullptr;
                if (layer && layer->isLocked()) continue;  // 锁定图层不删除
                manager->pushCommand(new RemoveShapeCommand(shape, scene, layer));
            }
        }
        manager->undoStack()->endMacro();
    } else {
        // 无 undo 支持时直接删除
        for (auto *item : items) {
            ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
            if (shape && !shape->parentItem()) {
                Layer *layer = doc ? doc->layerOf(shape) : nullptr;
                if (layer && layer->isLocked()) continue;
                scene->removeItem(shape);
                if (layer) layer->removeShape(shape);
                delete shape;
            }
        }
    }

    scene->setModified(true);
}
