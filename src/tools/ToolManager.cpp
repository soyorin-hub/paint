#include "ToolManager.h"
#include "ToolBase.h"
#include "canvas/CanvasScene.h"
#include <QUndoStack>
#include <QUndoCommand>
#include <QAction>

ToolManager::ToolManager(CanvasScene *scene, QUndoStack *undoStack, QObject *parent)
    : QObject(parent)
    , m_scene(scene)
    , m_undoStack(undoStack)
{
}

ToolManager::~ToolManager()
{
    qDeleteAll(m_tools);
}

void ToolManager::registerTool(ToolBase *tool)
{
    if (!tool) return;
    m_tools.append(tool);

    connect(tool, &ToolBase::statusMessage, this, &ToolManager::statusMessage);

    // 第一个注册的工具默认为当前工具
    if (!m_activeTool) {
        setActiveTool(m_tools.size() - 1);
    }
}

ToolBase *ToolManager::tool(int index) const
{
    if (index < 0 || index >= m_tools.size()) return nullptr;
    return m_tools[index];
}

ToolBase *ToolManager::tool(const QString &name) const
{
    for (auto *t : m_tools) {
        if (t->name() == name) return t;
    }
    return nullptr;
}

void ToolManager::setActiveTool(int index)
{
    ToolBase *newTool = tool(index);
    if (!newTool || newTool == m_activeTool) return;

    if (m_activeTool) {
        m_activeTool->deactivated();
    }

    m_activeTool = newTool;
    m_activeTool->activated();

    // 更新场景光标
    m_scene->setCursor(m_activeTool->cursor());

    emit activeToolChanged(m_activeTool);
}

void ToolManager::setActiveTool(const QString &name)
{
    ToolBase *t = tool(name);
    if (t) {
        setActiveTool(m_tools.indexOf(t));
    }
}

QAction *ToolManager::createToolAction(int index, QObject *parent)
{
    ToolBase *t = tool(index);
    if (!t) return nullptr;

    QAction *action = new QAction(t->icon(), t->name(), parent);
    action->setCheckable(true);
    action->setToolTip(QString("%1 (%2)").arg(t->name(), t->shortcut()));
    action->setData(index);

    connect(action, &QAction::triggered, this, [this, index]() {
        setActiveTool(index);
    });

    // 同步 checked 状态
    connect(this, &ToolManager::activeToolChanged, action, [this, action](ToolBase *tool) {
        int idx = m_tools.indexOf(tool);
        action->setChecked(idx == action->data().toInt());
    });

    return action;
}

// ============== 事件转发 ==============

void ToolManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->mousePressEvent(event, m_scene);
    }
}

void ToolManager::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->mouseMoveEvent(event, m_scene);
    }
}

void ToolManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_activeTool) {
        m_activeTool->mouseReleaseEvent(event, m_scene);
    }
}

void ToolManager::keyPressEvent(QKeyEvent *event)
{
    if (m_activeTool) {
        m_activeTool->keyPressEvent(event, m_scene);
    }
}

void ToolManager::pushCommand(QUndoCommand *cmd)
{
    if (m_undoStack && cmd) {
        m_undoStack->push(cmd);
    }
}
