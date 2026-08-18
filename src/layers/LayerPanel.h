#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include <QWidget>

class Document;
class Layer;
class QTreeWidgetItem;
class QTreeWidget;
class QLabel;

namespace Ui {
class LayerPanel;
}

class LayerPanel : public QWidget
{
    Q_OBJECT

public:
    explicit LayerPanel(QWidget *parent = nullptr);
    ~LayerPanel();

    void setDocument(Document *document);
    void refreshTree();
    void refreshGroupTree();

signals:
    void layerSelected(Layer *layer);
    void addLayerRequested();
    void removeLayerRequested(int index);
    void moveLayerRequested(int from, int to);
    void renameLayerRequested(int index, const QString &newName);
    void renameGroupRequested(qint64 groupId, const QString &newName);
    void groupSelected(qint64 groupId);

private slots:
    void onAddLayer();
    void onRemoveLayer();
    void onMoveUp();
    void onMoveDown();
    void onTreeItemClicked(QTreeWidgetItem *item, int column);
    void onVisibilityToggled(bool visible);
    void onLockToggled(bool locked);

private:
    Ui::LayerPanel *ui;
    Document *m_document = nullptr;
    QTreeWidget *m_groupTree = nullptr;
    QLabel *m_groupTitle = nullptr;
};

#endif // LAYERPANEL_H
