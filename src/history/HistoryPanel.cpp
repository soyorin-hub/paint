#include "HistoryPanel.h"
#include <QUndoStack>
#include <QUndoCommand>
#include <QListWidget>
#include <QVBoxLayout>
#include <QLabel>

HistoryPanel::HistoryPanel(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);
    layout->setSpacing(4);

    auto *title = new QLabel(tr("历史记录"));
    QFont f = title->font();
    f.setBold(true);
    f.setPointSize(11);
    title->setFont(f);
    layout->addWidget(title);

    m_list = new QListWidget();
    m_list->setSelectionMode(QAbstractItemView::SingleSelection);
    m_list->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(m_list);

    connect(m_list, &QListWidget::itemClicked, this, &HistoryPanel::onItemClicked);
}

void HistoryPanel::setUndoStack(QUndoStack *stack)
{
    if (m_undoStack) {
        disconnect(m_undoStack, nullptr, this, nullptr);
    }
    m_undoStack = stack;
    if (m_undoStack) {
        connect(m_undoStack, &QUndoStack::indexChanged,  this, &HistoryPanel::refreshList);
        connect(m_undoStack, &QUndoStack::cleanChanged,  this, &HistoryPanel::refreshList);
    }
    refreshList();
}

void HistoryPanel::refreshList()
{
    if (!m_undoStack) return;

    m_syncing = true;
    m_list->clear();

    int count = m_undoStack->count();
    int currentIdx = m_undoStack->index(); // 当前状态所在位置（即已执行的命令数）

    for (int i = 0; i < count; ++i) {
        QString text;
        // i < currentIdx 说明这个命令已经被"执行"
        // index() 返回的是当前在栈中的位置（redo 列表的起点）
        // 在 QUndoStack 中：
        //   index() = 已经 undo 后剩下的命令数
        //   已执行的命令在 [0, index())，未执行的（redo）在 [index(), count())

        int idx = i + 1; // 显示用，从 1 开始
        text = m_undoStack->text(i); // 命令描述文本
        if (text.isEmpty()) text = tr("操作 %1").arg(idx);

        QListWidgetItem *item = new QListWidgetItem(QString("%1. %2").arg(idx).arg(text));

        if (i < currentIdx) {
            // 已执行的操作：正常显示
            item->setForeground(QColor("#333"));
        } else {
            // 未执行（redo）的操作：变灰
            item->setForeground(QColor("#bbb"));
        }

        if (i == currentIdx - 1) {
            // 当前状态：加粗、高亮背景
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
            item->setBackground(QColor("#e3f2fd"));
        }

        m_list->addItem(item);
    }

    // 滚动到底部（最新操作）
    if (m_list->count() > 0)
        m_list->scrollToBottom();

    m_syncing = false;
}

void HistoryPanel::onItemClicked(QListWidgetItem *item)
{
    if (!m_undoStack || m_syncing) return;

    int row = m_list->row(item);
    if (row < 0) return;

    int targetIdx = row + 1; // 要导航到的位置（已执行的命令数）

    // 如果点在已执行区域（含当前）：undo 回去
    // 如果点在未执行（灰色）区域：redo 到那里
    int currentIdx = m_undoStack->index();

    if (targetIdx < currentIdx) {
        // 回退：撤销到目标位置
        int steps = currentIdx - targetIdx;
        for (int i = 0; i < steps; ++i)
            m_undoStack->undo();
    } else if (targetIdx > currentIdx) {
        // 前进：重做到目标位置
        int steps = targetIdx - currentIdx;
        for (int i = 0; i < steps; ++i)
            m_undoStack->redo();
    }
    // targetIdx == currentIdx：无需操作，但点击当前项也不用做什么
}
