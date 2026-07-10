#include "LayerPanel.h"
#include "ui_LayerPanel.h"
#include "Layer.h"
#include "document/Document.h"
#include <QListWidgetItem>

LayerPanel::LayerPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LayerPanel)
{
    ui->setupUi(this);

    connect(ui->addBtn,    &QPushButton::clicked, this, &LayerPanel::onAddLayer);
    connect(ui->removeBtn, &QPushButton::clicked, this, &LayerPanel::onRemoveLayer);
    connect(ui->upBtn,     &QPushButton::clicked, this, &LayerPanel::onMoveUp);
    connect(ui->downBtn,   &QPushButton::clicked, this, &LayerPanel::onMoveDown);

    connect(ui->layerList, &QListWidget::currentRowChanged,
            this, &LayerPanel::onLayerListChanged);

    connect(ui->visibleCheck, &QCheckBox::toggled,
            this, &LayerPanel::onVisibilityToggled);
    connect(ui->lockCheck, &QCheckBox::toggled,
            this, &LayerPanel::onLockToggled);
}

LayerPanel::~LayerPanel()
{
    delete ui;
}

void LayerPanel::setDocument(Document *document)
{
    m_document = document;
    if (!m_document) return;

    connect(m_document, &Document::layerAdded, this, [this](Layer *, int) {
        refreshList();
    });
    connect(m_document, &Document::layerRemoved, this, [this](int) {
        refreshList();
    });
    connect(m_document, &Document::layerMoved, this, [this](int, int) {
        refreshList();
    });
    connect(m_document, &Document::activeLayerChanged, this, [this](Layer *) {
        // 同步列表选中
        int idx = m_document->activeLayerIndex();
        if (ui->layerList->currentRow() != idx) {
            ui->layerList->blockSignals(true);
            ui->layerList->setCurrentRow(idx);
            ui->layerList->blockSignals(false);
        }
        // 更新复选框
        Layer *layer = m_document->activeLayer();
        if (layer) {
            ui->visibleCheck->setChecked(layer->isVisible());
            ui->lockCheck->setChecked(layer->isLocked());
        }
    });

    refreshList();
}

void LayerPanel::refreshList()
{
    if (!m_document) return;

    ui->layerList->blockSignals(true);
    ui->layerList->clear();

    for (int i = m_document->layerCount() - 1; i >= 0; --i) {
        Layer *layer = m_document->layers()[i];
        QListWidgetItem *item = new QListWidgetItem(layer->name());
        item->setData(Qt::UserRole, i);
        if (i == m_document->activeLayerIndex()) {
            item->setSelected(true);
            QFont f = item->font();
            f.setBold(true);
            item->setFont(f);
        }
        ui->layerList->addItem(item);
    }

    ui->layerList->blockSignals(false);

    // 刷新后同步选中
    int idx = m_document->activeLayerIndex();
    int listRow = m_document->layerCount() - 1 - idx;
    if (ui->layerList->currentRow() != listRow) {
        ui->layerList->setCurrentRow(listRow);
    }
}

void LayerPanel::updateLayerItem(int index)
{
    Q_UNUSED(index)
    refreshList();
}

void LayerPanel::onAddLayer()
{
    if (!m_document) return;
    m_document->addLayer();
    refreshList();
}

void LayerPanel::onRemoveLayer()
{
    if (!m_document) return;
    int row = ui->layerList->currentRow();
    if (row < 0) return;
    int idx = m_document->layerCount() - 1 - row;
    m_document->removeLayer(idx);
}

void LayerPanel::onMoveUp()
{
    if (!m_document) return;
    int row = ui->layerList->currentRow();
    if (row <= 0) return;
    int idx = m_document->layerCount() - 1 - row;
    m_document->moveLayer(idx, idx + 1); // 上移 = 索引 +1（显示靠上 = 更高 z-order）
}

void LayerPanel::onMoveDown()
{
    if (!m_document) return;
    int row = ui->layerList->currentRow();
    if (row < 0 || row >= m_document->layerCount() - 1) return;
    int idx = m_document->layerCount() - 1 - row;
    m_document->moveLayer(idx, idx - 1);
}

void LayerPanel::onLayerListChanged(int row)
{
    if (!m_document || row < 0) return;
    int idx = m_document->layerCount() - 1 - row;
    m_document->setActiveLayer(idx);
    emit layerSelected(m_document->activeLayer());
}

void LayerPanel::onVisibilityToggled(bool visible)
{
    Layer *layer = m_document ? m_document->activeLayer() : nullptr;
    if (layer) layer->setVisible(visible);
}

void LayerPanel::onLockToggled(bool locked)
{
    Layer *layer = m_document ? m_document->activeLayer() : nullptr;
    if (layer) layer->setLocked(locked);
}
