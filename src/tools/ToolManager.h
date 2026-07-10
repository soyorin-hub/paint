#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

#include <QObject>
#include <QList>

class ToolBase;
class CanvasScene;
class QGraphicsSceneMouseEvent;
class QKeyEvent;
class QAction;
class QUndoStack;
class QUndoCommand;

class ToolManager : public QObject
{
    Q_OBJECT

public:
    explicit ToolManager(CanvasScene *scene, QUndoStack *undoStack, QObject *parent = nullptr);
    ~ToolManager();

    // 工具注册
    void registerTool(ToolBase *tool);
    ToolBase *tool(int index) const;
    ToolBase *tool(const QString &name) const;
    int toolCount() const { return m_tools.size(); }

    // 当前工具
    void setActiveTool(int index);
    void setActiveTool(const QString &name);
    ToolBase *activeTool() const { return m_activeTool; }
    int activeToolIndex() const { return m_tools.indexOf(m_activeTool); }

    // 创建工具 Action（用于工具栏/菜单）
    QAction *createToolAction(int index, QObject *parent = nullptr);

    // 撤销/重做
    QUndoStack *undoStack() const { return m_undoStack; }
    void pushCommand(QUndoCommand *cmd);

    // 事件转发（由 CanvasScene 调用）
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    void activeToolChanged(ToolBase *tool);
    void statusMessage(const QString &msg);

private:
    CanvasScene *m_scene = nullptr;
    QUndoStack  *m_undoStack = nullptr;
    QList<ToolBase*> m_tools;
    ToolBase *m_activeTool = nullptr;
};

#endif // TOOLMANAGER_H
