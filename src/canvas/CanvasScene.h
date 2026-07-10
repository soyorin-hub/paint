#ifndef CANVASSCENE_H
#define CANVASSCENE_H

#include <QGraphicsScene>
#include <QCursor>

class ToolManager;
class QUndoCommand;

class CanvasScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CanvasScene(QObject *parent = nullptr);

    void setToolManager(ToolManager *manager);
    ToolManager *toolManager() const { return m_toolManager; }

    bool isModified() const { return m_modified; }
    void setModified(bool modified);

    void setCursor(const QCursor &c) { m_cursor = c; }
    QCursor cursor() const { return m_cursor; }

    void pushUndoCommand(QUndoCommand *cmd);

signals:
    void sceneModified();
    void itemSelected(QGraphicsItem *item);
    void itemDeselected();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    bool m_modified = false;
    ToolManager *m_toolManager = nullptr;
    QCursor m_cursor;
};

#endif
