#ifndef SELECTTOOL_H
#define SELECTTOOL_H

#include "ToolBase.h"
#include "shapes/ShapeBase.h"
#include <QList>

class QGraphicsItem;
class QGraphicsRectItem;

class SelectTool : public ToolBase
{
    Q_OBJECT

public:
    explicit SelectTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("选择"); }
    QIcon icon() const override { return QIcon(":/icons/select.svg"); }
    QCursor cursor() const override { return Qt::ArrowCursor; }
    QString shortcut() const override { return "V"; }
    bool worksOnLockedLayer() const override { return true; }

private:
    HandleType handleAt(const QPointF &scenePos, ShapeBase *shape) const;
    void applyResize(ShapeBase *shape, HandleType handle, const QPointF &newScenePos);
    void dragLineEndpoint(ShapeBase *shape, int handleIndex, const QPointF &scenePos);

    // 根据手柄类型返回对应的双向箭头光标
    static QCursor cursorForHandle(HandleType h);
    // 根据鼠标位置更新视口光标
    void updateCursor(CanvasScene *scene, const QPointF &scenePos);
    // 辅助：在 scene 关联的 viewport 上设置光标
    static void setViewCursor(CanvasScene *scene, const QCursor &cursor);

    bool m_movingItem = false;
    bool m_handleDragging = false;
    HandleType m_activeHandle = Handle_None;
    QPointF m_lastScenePos;
    QPointF m_dragStartPos;
    QGraphicsItem *m_activeItem = nullptr;
    QRectF m_originalRect;   // resize 前的原始矩形
    QPointF m_originalPos;   // resize/move 前的原始位置
    QTransform m_originalTransform; // move 前的原始变换
    QLineF m_originalLine;   // 端点拖拽前的原始线段
    QPointF m_originalCenter; // 端点拖拽前的原始中心点
    QSizeF m_originalSize;   // 矩形类缩放前的原始尺寸
    qreal m_originalRotate = 0;

    // 多选移动（含编组联动）
    QList<QGraphicsItem*> m_moveItems;
    QList<QPointF> m_moveOrigPos;
    QList<QTransform> m_moveOrigXf;

    // 框选（marquee）
    bool m_marqueeSelecting = false;
    bool m_marqueeAdd = false;
    QPointF m_marqueeStart;
    QGraphicsRectItem *m_marqueeRect = nullptr;
};

#endif // SELECTTOOL_H
