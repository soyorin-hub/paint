#include "LayerPanel.h"
#include "ui_LayerPanel.h"
#include "Layer.h"
#include "document/Document.h"
#include "shapes/ShapeBase.h"
#include <QTreeWidgetItem>
#include <QTreeWidget>
#include <QPushButton>
#include <QGraphicsScene>
#include <QIcon>
#include <QLabel>
#include <QAbstractItemView>

LayerPanel::LayerPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LayerPanel)
{
    ui->setupUi(this);

    connect(ui->addBtn,    &QPushButton::clicked, this, &LayerPanel::onAddLayer);
    connect(ui->removeBtn, &QPushButton::clicked, this, &LayerPanel::onRemoveLayer);
    connect(ui->upBtn,     &QPushButton::clicked, this, &LayerPanel::onMoveUp);
    connect(ui->downBtn,   &QPushButton::clicked, this, &LayerPanel::onMoveDown);

    connect(ui->layerTree, &QTreeWidget::itemClicked,
            this, &LayerPanel::onTreeItemClicked);
    connect(ui->layerTree, &QTreeWidget::itemChanged,
            this, [this](QTreeWidgetItem *item, int column) {
        if (column != 0 || !m_document || !item) return;
        QString newName = item->text(0);
        int shapeIdx = item->data(0, Qt::UserRole + 1).toInt();

        if (shapeIdx < 0) {
            // 图层改名
            int layerIdx = item->data(0, Qt::UserRole).toInt();
            if (layerIdx >= 0 && layerIdx < m_document->layerCount())
                emit renameLayerRequested(layerIdx, newName);
        } else {
            // 图形改名
            qintptr ptr = item->data(0, Qt::UserRole + 2).value<qintptr>();
            ShapeBase *shape = reinterpret_cast<ShapeBase*>(ptr);
            if (shape) {
                shape->setShapeName(newName);
                refreshGroupTree();  // 同步到组面板
            }
        }
    });

    connect(ui->visibleCheck, &QCheckBox::toggled,
            this, &LayerPanel::onVisibilityToggled);
    connect(ui->lockCheck, &QCheckBox::toggled,
            this, &LayerPanel::onLockToggled);

    // ===== 组栏目 =====
    m_groupTitle = new QLabel(tr("组"));
    QFont gf = m_groupTitle->font();
    gf.setBold(true);
    gf.setPointSize(11);
    m_groupTitle->setFont(gf);

    m_groupTree = new QTreeWidget();
    m_groupTree->setColumnCount(1);
    m_groupTree->setHeaderHidden(true);
    m_groupTree->setIndentation(16);
    m_groupTree->setRootIsDecorated(false);
    m_groupTree->setSelectionMode(QAbstractItemView::SingleSelection);
    m_groupTree->setFocusPolicy(Qt::NoFocus);
    m_groupTree->setMinimumHeight(80);

    ui->mainLayout->addWidget(m_groupTitle);
    ui->mainLayout->addWidget(m_groupTree);

    // 组节点点击 → 选中整组；图形节点点击 → 选中该图形
    connect(m_groupTree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item, int) {
        if (!m_document || !item) return;
        qint64 gid = item->data(0, Qt::UserRole).toLongLong();
        bool isGroup = item->data(0, Qt::UserRole + 1).toBool();
        QGraphicsScene *scene = nullptr;
        QList<ShapeBase*> members;
        for (auto *layer : m_document->layers()) {
            for (auto *shape : layer->shapes()) {
                if (shape->groupId() == gid) {
                    members.append(shape);
                    if (!scene) scene = shape->scene();
                }
            }
        }
        if (!scene) return;
        if (isGroup) {
            scene->clearSelection();
            for (auto *s : members) s->setSelected(true);
            emit groupSelected(gid);
        } else {
            ShapeBase *shape = reinterpret_cast<ShapeBase*>(
                item->data(0, Qt::UserRole + 2).value<qintptr>());
            if (shape) {
                scene->clearSelection();
                shape->setSelected(true);
            }
        }
    });

    // 组节点改名 / 组内图形改名
    connect(m_groupTree, &QTreeWidget::itemChanged, this, [this](QTreeWidgetItem *item, int column) {
        if (column != 0 || !m_document || !item) return;
        if (item->data(0, Qt::UserRole + 1).toBool()) {
            // 组节点改名
            qint64 gid = item->data(0, Qt::UserRole).toLongLong();
            emit renameGroupRequested(gid, item->text(0));
        } else {
            // 图形改名 → 同步到图层面板
            ShapeBase *shape = reinterpret_cast<ShapeBase*>(
                item->data(0, Qt::UserRole + 2).value<qintptr>());
            if (shape) {
                shape->setShapeName(item->text(0));
                refreshTree();
            }
        }
    });
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
        refreshTree();
    });
    connect(m_document, &Document::layerRemoved, this, [this](int) {
        refreshTree();
    });
    connect(m_document, &Document::layerMoved, this, [this](int, int) {
        refreshTree();
    });
    connect(m_document, &Document::activeLayerChanged, this, [this](Layer *layer) {
        // 同步复选框
        if (layer) {
            ui->visibleCheck->setChecked(layer->isVisible());
            ui->lockCheck->setChecked(layer->isLocked());
        }
        refreshTree();
    });
    connect(m_document, &Document::modified, this, [this]() {
        refreshTree();
        refreshGroupTree();  // 图形增删会影响组成员
    });
    connect(m_document, &Document::groupsChanged, this, [this]() {
        refreshGroupTree();
    });

    refreshTree();
    refreshGroupTree();
}

void LayerPanel::refreshTree()
{
    if (!m_document) return;

    ui->layerTree->blockSignals(true);
    ui->layerTree->clear();
    ui->layerTree->setColumnCount(2);
    ui->layerTree->setColumnWidth(0, 100);
    ui->layerTree->setColumnWidth(1, 28);
    ui->layerTree->header()->setStretchLastSection(false);

    int activeIdx = m_document->activeLayerIndex();

    // 从上层往下层显示（倒序遍历）
    for (int i = m_document->layerCount() - 1; i >= 0; --i) {
        Layer *layer = m_document->layers()[i];

        // 图层节点
        QTreeWidgetItem *layerItem = new QTreeWidgetItem();
        layerItem->setText(0, layer->name());
        layerItem->setData(0, Qt::UserRole, i);           // 图层索引
        layerItem->setData(0, Qt::UserRole + 1, -1);      // -1 表示图层节点
        layerItem->setFlags(layerItem->flags() | Qt::ItemIsEditable);
        if (i == activeIdx) {
            QFont f = layerItem->font(0);
            f.setBold(true);
            layerItem->setFont(0, f);
        }
        ui->layerTree->addTopLevelItem(layerItem);

        // 该图层的图形子节点
        const auto &shapes = layer->shapes();
        for (int s = 0; s < shapes.size(); ++s) {
            ShapeBase *shape = shapes[s];

            QTreeWidgetItem *shapeItem = new QTreeWidgetItem();
            QString displayName = shape->shapeName().isEmpty()
                ? tr("形状 %1").arg(s + 1) : shape->shapeName();
            shapeItem->setText(0, displayName);
            shapeItem->setData(0, Qt::UserRole, s);                  // 图形在图层中的索引
            shapeItem->setData(0, Qt::UserRole + 1, i);              // 所属图层索引
            shapeItem->setData(0, Qt::UserRole + 2,
                               reinterpret_cast<qintptr>(shape));     // 图形指针
            shapeItem->setFlags(shapeItem->flags() | Qt::ItemIsEditable);
            layerItem->addChild(shapeItem);

            // 可见性切换按钮（放在第 2 列，即行末）
            QPushButton *visBtn = new QPushButton();
            visBtn->setCheckable(true);
            visBtn->setChecked(shape->isVisible());
            visBtn->setFixedSize(22, 22);
            visBtn->setIconSize(QSize(16, 16));
            visBtn->setToolTip(tr("显示/隐藏"));
            visBtn->setStyleSheet(
                "QPushButton { border: none; background: transparent; padding: 0; }");

            auto updateBtn = [visBtn](bool visible) {
                visBtn->setIcon(visible ? QIcon(":/icons/eye.svg") : QIcon());
            };
            updateBtn(shape->isVisible());

            connect(visBtn, &QPushButton::toggled, this, [shape, updateBtn](bool checked) {
                shape->setVisible(checked);
                updateBtn(checked);
            });

            ui->layerTree->setItemWidget(shapeItem, 1, visBtn);
        }
    }

    // 选中当前活跃图层
    for (int i = 0; i < ui->layerTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = ui->layerTree->topLevelItem(i);
        if (item->data(0, Qt::UserRole).toInt() == activeIdx) {
            ui->layerTree->setCurrentItem(item);
            break;
        }
    }

    ui->layerTree->blockSignals(false);
    ui->layerTree->expandAll();
}

void LayerPanel::refreshGroupTree()
{
    if (!m_document || !m_groupTree) return;

    m_groupTree->blockSignals(true);
    m_groupTree->clear();

    for (qint64 gid : m_document->groupIds()) {
        // 收集该组成员
        QList<ShapeBase*> members;
        for (auto *layer : m_document->layers()) {
            for (auto *shape : layer->shapes()) {
                if (shape->groupId() == gid)
                    members.append(shape);
            }
        }
        if (members.isEmpty()) continue;  // 无成员的组名不显示

        QString name = m_document->groupName(gid);
        if (name.isEmpty()) name = tr("组 %1").arg(gid);

        QTreeWidgetItem *groupItem = new QTreeWidgetItem();
        groupItem->setText(0, name);
        groupItem->setData(0, Qt::UserRole, gid);
        groupItem->setData(0, Qt::UserRole + 1, true);  // 组节点
        groupItem->setFlags(groupItem->flags() | Qt::ItemIsEditable);
        m_groupTree->addTopLevelItem(groupItem);

        for (auto *shape : members) {
            QTreeWidgetItem *shapeItem = new QTreeWidgetItem();
            QString displayName = shape->shapeName().isEmpty()
                ? tr("形状") : shape->shapeName();
            shapeItem->setText(0, displayName);
            shapeItem->setData(0, Qt::UserRole, gid);
            shapeItem->setData(0, Qt::UserRole + 1, false);  // 图形节点
            shapeItem->setData(0, Qt::UserRole + 2,
                               reinterpret_cast<qintptr>(shape));
            shapeItem->setFlags(shapeItem->flags() | Qt::ItemIsEditable);
            groupItem->addChild(shapeItem);
        }
    }

    m_groupTree->blockSignals(false);
    m_groupTree->expandAll();
}

void LayerPanel::onAddLayer()
{
    if (!m_document) return;
    emit addLayerRequested();
}

void LayerPanel::onRemoveLayer()
{
    if (!m_document) return;
    QTreeWidgetItem *item = ui->layerTree->currentItem();
    if (!item) return;
    // 如果选中的是图形子节点，找到其图层父节点
    if (item->data(0, Qt::UserRole + 1).toInt() >= 0)
        item = item->parent();
    if (!item) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    emit removeLayerRequested(idx);
}

void LayerPanel::onMoveUp()
{
    if (!m_document) return;
    QTreeWidgetItem *item = ui->layerTree->currentItem();
    if (!item || item->data(0, Qt::UserRole + 1).toInt() >= 0) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    if (idx >= m_document->layerCount() - 1) return;
    emit moveLayerRequested(idx, idx + 1);
}

void LayerPanel::onMoveDown()
{
    if (!m_document) return;
    QTreeWidgetItem *item = ui->layerTree->currentItem();
    if (!item || item->data(0, Qt::UserRole + 1).toInt() >= 0) return;
    int idx = item->data(0, Qt::UserRole).toInt();
    if (idx <= 0) return;
    emit moveLayerRequested(idx, idx - 1);
}

void LayerPanel::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if (!m_document || !item) return;

    int shapeIndex = item->data(0, Qt::UserRole + 1).toInt(); // -1 = layer, >=0 = shape
    if (shapeIndex < 0) {
        // 图层节点：切换活跃图层
        int layerIdx = item->data(0, Qt::UserRole).toInt();
        m_document->setActiveLayer(layerIdx);
        emit layerSelected(m_document->activeLayer());
    } else {
        // 图形子节点：在画布上选中该图形
        qintptr ptr = item->data(0, Qt::UserRole + 2).value<qintptr>();
        ShapeBase *shape = reinterpret_cast<ShapeBase*>(ptr);
        if (shape && shape->scene()) {
            shape->scene()->clearSelection();
            shape->setSelected(true);
        }
    }
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
