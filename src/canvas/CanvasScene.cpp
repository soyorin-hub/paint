#include "CanvasScene.h"
#include "tools/ToolManager.h"
#include "tools/ToolBase.h"
#include "document/Document.h"
#include "layers/Layer.h"
#include "shapes/TextShape.h"
#include "commands/TextCommand.h"
#include <QGraphicsItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QTextCursor>
#include <QUndoStack>

CanvasScene::CanvasScene(QObject *parent) : QGraphicsScene(parent)
{
    setSceneRect(-2000, -2000, 4000, 4000);
    connect(this, &QGraphicsScene::selectionChanged, this, [this]() {
        auto sel = selectedItems();
        if (sel.isEmpty()) emit itemDeselected();
        else emit itemSelected(sel.first());
    });

    // 文字编辑器失去焦点时提交
    connect(this, &QGraphicsScene::focusItemChanged, this,
            [this](QGraphicsItem *, QGraphicsItem *oldFocus, Qt::FocusReason) {
        if (m_textEditor && oldFocus == m_textEditor)
            commitTextEditing();
    });
}

void CanvasScene::setToolManager(ToolManager *m) { m_toolManager = m; }
void CanvasScene::setModified(bool m) { if (m_modified != m) { m_modified = m; if (m) emit sceneModified(); } }
void CanvasScene::pushUndoCommand(QUndoCommand *cmd) { if (m_toolManager) m_toolManager->pushCommand(cmd); }

void CanvasScene::beginUndoMacro(const QString &text)
{
    if (m_toolManager && m_toolManager->undoStack())
        m_toolManager->undoStack()->beginMacro(text);
}

void CanvasScene::endUndoMacro()
{
    if (m_toolManager && m_toolManager->undoStack())
        m_toolManager->undoStack()->endMacro();
}

Layer *CanvasScene::activeLayer() const
{
    return m_document ? m_document->activeLayer() : nullptr;
}

// ===== 鼠标事件（编辑文字时交给编辑器，否则转发给工具） =====

void CanvasScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_textEditor) { QGraphicsScene::mousePressEvent(event); return; }
    // 活动图层锁定时禁止绘制/擦除（选择工具除外）
    if (m_document && m_document->activeLayer() && m_document->activeLayer()->isLocked()
        && m_toolManager && m_toolManager->activeTool()
        && !m_toolManager->activeTool()->worksOnLockedLayer()) {
        return;
    }
    if (m_toolManager) m_toolManager->mousePressEvent(event);
}

void CanvasScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_textEditor) { QGraphicsScene::mouseMoveEvent(event); return; }
    if (m_toolManager) m_toolManager->mouseMoveEvent(event);
}

void CanvasScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_textEditor) { QGraphicsScene::mouseReleaseEvent(event); return; }
    if (m_toolManager) m_toolManager->mouseReleaseEvent(event);
}

void CanvasScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (auto *text = dynamic_cast<TextShape*>(item)) {
        startTextEditing(text);
        return;
    }
    QGraphicsScene::mouseDoubleClickEvent(event);
}

// ===== 文字就地编辑 =====

void CanvasScene::startTextEditing(TextShape *shape)
{
    if (!shape) return;
    commitTextEditing(); // 若已在编辑其它文字，先提交

    m_editingShape = shape;
    m_textEditor = new QGraphicsTextItem();

    m_textEditor->setPlainText(shape->text());
    m_textEditor->setFont(shape->font());

    QColor textColor = shape->shapeStyle().strokeColor;
    if (!textColor.isValid() || textColor.alpha() == 0)
        textColor = Qt::black;
    m_textEditor->setDefaultTextColor(textColor);

    // 对齐到文字实际绘制起点（近似）
    QPointF originLocal = shape->boundingRect().topLeft() + QPointF(4, 2);
    m_textEditor->setPos(shape->mapToScene(originLocal) - QPointF(4, 4));

    m_textEditor->setTextInteractionFlags(Qt::TextEditorInteraction);
    m_textEditor->setZValue(10001);
    addItem(m_textEditor);

    // 编辑期间隐藏原形状，避免重叠显示
    shape->setVisible(false);

    m_textEditor->setFocus();
    QTextCursor cursor = m_textEditor->textCursor();
    cursor.select(QTextCursor::Document);
    m_textEditor->setTextCursor(cursor);
}

void CanvasScene::commitTextEditing()
{
    if (!m_textEditor || !m_editingShape) return;

    TextShape *shape = m_editingShape;
    QString newText = m_textEditor->toPlainText();

    QGraphicsTextItem *editor = m_textEditor;
    m_textEditor = nullptr;
    m_editingShape = nullptr;
    removeItem(editor);
    delete editor;

    shape->setVisible(true);

    if (newText != shape->text())
        pushUndoCommand(new TextCommand(shape, shape->text(), newText));

    setModified(true);
}
