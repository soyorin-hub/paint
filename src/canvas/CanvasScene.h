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

    // 文字就地编辑
    void startTextEditing(TextShape *shape);
    void commitTextEditing();
    bool isTextEditing() const { return m_textEditor != nullptr; }

signals:
    void sceneModified();
    void itemSelected(QGraphicsItem *item);
    void itemDeselected();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool m_modified = false;
    ToolManager *m_toolManager = nullptr;
    Document *m_document = nullptr;
    QCursor m_cursor;

    QGraphicsTextItem *m_textEditor = nullptr;   // 文字编辑时的临时编辑器
    TextShape *m_editingShape = nullptr;         // 正在被编辑的文字形状
};

#endif
