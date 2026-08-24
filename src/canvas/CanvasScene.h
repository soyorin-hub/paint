#ifndef CANVASSCENE_H
#define CANVASSCENE_H

#include <QGraphicsScene>
#include <QCursor>

class ToolManager;
class QUndoCommand;
class Document;
class Layer;
class TextShape;
class QGraphicsTextItem;

// 尺子（约束）指南：一条水平或垂直线，作为绘制/拉伸的边界
struct RulerGuide {
    Qt::Orientation orientation = Qt::Horizontal;
    qreal position = 0.0;   // 水平 = 场景 y；垂直 = 场景 x
    bool visible = false;
};

class CanvasScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CanvasScene(QObject *parent = nullptr);

    void setToolManager(ToolManager *manager);
    ToolManager *toolManager() const { return m_toolManager; }

    void setDocument(Document *document) { m_document = document; }
    Document *document() const { return m_document; }
    Layer *activeLayer() const;

    bool isModified() const { return m_modified; }
    void setModified(bool modified);

    void setCursor(const QCursor &c) { m_cursor = c; }
    QCursor cursor() const { return m_cursor; }

    void pushUndoCommand(QUndoCommand *cmd);
    void beginUndoMacro(const QString &text);
    void endUndoMacro();

    // 尺子（约束）指南
    RulerGuide ruler() const { return m_ruler; }
    void setRuler(const RulerGuide &r);
    void setRulerPosition(qreal pos);
    void setRulerOrientation(Qt::Orientation orient);
    void setRulerVisible(bool visible);
    void toggleRulerOrientation();
    // 尺子约束：把场景点钳制到 ref 所在一侧（无可见尺子时原样返回）
    QPointF constrainToRuler(const QPointF &scenePos, const QPointF &refScenePos) const;

    // 文字就地编辑
    void startTextEditing(TextShape *shape);
    void commitTextEditing();
    bool isTextEditing() const { return m_textEditor != nullptr; }

signals:
    void sceneModified();
    void itemSelected(QGraphicsItem *item);
    void itemDeselected();
    void rulerChanged();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

private:
    bool m_modified = false;
    ToolManager *m_toolManager = nullptr;
    Document *m_document = nullptr;
    QCursor m_cursor;
    RulerGuide m_ruler;

    QGraphicsTextItem *m_textEditor = nullptr;   // 文字编辑时的临时编辑器
    TextShape *m_editingShape = nullptr;         // 正在被编辑的文字形状
};

#endif
