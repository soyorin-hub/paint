#ifndef LAYERPANEL_H
#define LAYERPANEL_H

#include <QWidget>

class Document;
class Layer;
class QListWidgetItem;

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

signals:
    void layerSelected(Layer *layer);

private slots:
    void onAddLayer();
    void onRemoveLayer();
    void onMoveUp();
    void onMoveDown();
    void onLayerListChanged(int row);
    void onVisibilityToggled(bool visible);
    void onLockToggled(bool locked);

private:
    void refreshList();
    void updateLayerItem(int index);
    void blockSignals();

    Ui::LayerPanel *ui;
    Document *m_document = nullptr;
};

#endif // LAYERPANEL_H
