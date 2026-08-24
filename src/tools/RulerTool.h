#ifndef RULERTOOL_H
#define RULERTOOL_H

#include "ToolBase.h"

// 尺子工具：拖动放置/移动一条水平或垂直的约束线
//  拖动 = 移动；O = 切换横/竖；H = 显示/隐藏
class RulerTool : public ToolBase
{
    Q_OBJECT
public:
    explicit RulerTool(QObject *parent = nullptr);

    void mousePressEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event, CanvasScene *scene) override;
    void keyPressEvent(QKeyEvent *event, CanvasScene *scene) override;

    QString name() const override { return tr("尺子"); }
    QIcon icon() const override { return QIcon(":/icons/ruler.svg"); }
    QCursor cursor() const override { return Qt::CrossCursor; }
    QString shortcut() const override { return "R"; }
    bool worksOnLockedLayer() const override { return true; }

private:
    bool m_dragging = false;
};

#endif // RULERTOOL_H
