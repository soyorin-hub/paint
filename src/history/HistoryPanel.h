#ifndef HISTORYPANEL_H
#define HISTORYPANEL_H

#include <QWidget>

class QUndoStack;
class QListWidget;
class QListWidgetItem;

class HistoryPanel : public QWidget
{
    Q_OBJECT

public:
    explicit HistoryPanel(QWidget *parent = nullptr);

    void setUndoStack(QUndoStack *stack);

private slots:
    void refreshList();
    void onItemClicked(QListWidgetItem *item);

private:
    QListWidget *m_list;
    QUndoStack *m_undoStack = nullptr;
    bool m_syncing = false;
};

#endif // HISTORYPANEL_H
