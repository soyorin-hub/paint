#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QUndoStack>
#include <QApplication>
#include <QShortcut>
#include <QActionGroup>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QFontComboBox>
#include <QHBoxLayout>

#include "canvas/CanvasView.h"
#include "canvas/CanvasScene.h"
#include "tools/ToolManager.h"
#include "tools/SelectTool.h"
#include "tools/RectTool.h"
#include "tools/EllipseTool.h"
#include "tools/LineTool.h"
#include "tools/FreehandTool.h"
#include "tools/TextTool.h"
#include "tools/EraserTool.h"
#include "document/Document.h"
#include "layers/Layer.h"
#include "layers/LayerPanel.h"
#include "properties/PropertyPanel.h"
#include "shapes/ShapeBase.h"
#include "shapes/TextShape.h"
#include "document/FileManager.h"
#include "commands/RemoveShapeCommand.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_undoStack = new QUndoStack(this);

    // 创建画布
    m_canvasScene = new CanvasScene(this);
    m_canvasView  = new CanvasView(this);
    m_canvasView->setScene(m_canvasScene);

    // 创建工具管理器
    m_toolManager = new ToolManager(m_canvasScene, m_undoStack, this);
    m_canvasScene->setToolManager(m_toolManager);

    // 注册工具: 0=选择 1=矩形 2=椭圆 3=线段 4=画笔 5=文字 6=橡皮擦
    m_toolManager->registerTool(new SelectTool(this));
    m_toolManager->registerTool(new RectTool(this));
    m_toolManager->registerTool(new EllipseTool(this));
    m_toolManager->registerTool(new LineTool(this));
    m_toolManager->registerTool(new FreehandTool(this));
    m_toolManager->registerTool(new TextTool(this));
    m_toolManager->registerTool(new EraserTool(this));
    m_toolManager->setActiveTool(0);

    // 创建文档和文件管理器
    m_document = new Document(this);
    m_fileManager = new FileManager(this);
    m_document->applyToScene(m_canvasScene);

    setCentralWidget(m_canvasView);

    setupMenuBar();
    setupToolBars();
    setupDockWidgets();
    setupStatusBar();
    setupConnections();
    updateWindowTitle();

    setWindowIcon(QIcon(":/icons/app.svg"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUi() {}

void MainWindow::setupMenuBar()
{
    // 文件菜单
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));
    QAction *newAction = fileMenu->addAction(tr("新建(&N)"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewFile);
    QAction *openAction = fileMenu->addAction(tr("打开(&O)..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::onOpenFile);
    fileMenu->addSeparator();
    QAction *saveAction = fileMenu->addAction(tr("保存(&S)"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::onSaveFile);
    QAction *saveAsAction = fileMenu->addAction(tr("另存为(&A)..."));
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    fileMenu->addSeparator();
    QMenu *exportMenu = fileMenu->addMenu(tr("导出"));
    QAction *exportSvgAction = exportMenu->addAction(tr("导出 SVG..."));
    connect(exportSvgAction, &QAction::triggered, this, &MainWindow::onExportSvg);
    QAction *exportPngAction = exportMenu->addAction(tr("导出 PNG..."));
    connect(exportPngAction, &QAction::triggered, this, &MainWindow::onExportPng);
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction(tr("退出(&X)"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // 编辑菜单
    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    QAction *undoAction = m_undoStack->createUndoAction(this, tr("撤销(&U)"));
    undoAction->setShortcut(QKeySequence::Undo);
    editMenu->addAction(undoAction);
    QAction *redoAction = m_undoStack->createRedoAction(this, tr("重做(&R)"));
    redoAction->setShortcut(QKeySequence::Redo);
    editMenu->addAction(redoAction);
    editMenu->addSeparator();
    QAction *deleteAction = editMenu->addAction(tr("删除(&D)"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    QAction *selectAllAction = editMenu->addAction(tr("全选(&A)"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::onSelectAll);

    // 视图菜单
    QMenu *viewMenu = menuBar()->addMenu(tr("视图(&V)"));
    QAction *zoomInAction = viewMenu->addAction(tr("放大(&I)"));
    zoomInAction->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAction, &QAction::triggered, m_canvasView, &CanvasView::zoomIn);
    QAction *zoomOutAction = viewMenu->addAction(tr("缩小(&O)"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAction, &QAction::triggered, m_canvasView, &CanvasView::zoomOut);
    QAction *zoomFitAction = viewMenu->addAction(tr("适合窗口(&F)"));
    zoomFitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));
    connect(zoomFitAction, &QAction::triggered, m_canvasView, &CanvasView::zoomFit);
    // 背景选择
    QMenu *bgMenu = viewMenu->addMenu(tr("画布背景"));
    QActionGroup *bgGroup = new QActionGroup(this);
    bgGroup->setExclusive(true);
    auto addBgAction = [&](const QString &name, int type) {
        QAction *a = bgMenu->addAction(name);
        a->setCheckable(true);
        bgGroup->addAction(a);
        connect(a, &QAction::triggered, this, [this, type]() {
            m_canvasView->setBackgroundType(type);
        });
        if (type == 2) a->setChecked(true); // 默认点阵
    };
    addBgAction(tr("纯白"), 0);
    addBgAction(tr("格子"), 1);
    addBgAction(tr("点阵"), 2);

    // 帮助
    QMenu *helpMenu = menuBar()->addMenu(tr("帮助(&H)"));
    QAction *aboutAction = helpMenu->addAction(tr("关于(&A)"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("关于 Paint"),
            tr("<h3>Paint 轻量级矢量画板</h3>"
               "<p>版本 1.0.0</p><p>基于 Qt6 的轻量级矢量图形绘制工具。</p>"));
    });
}

void MainWindow::setupToolBars()
{
    // ===== 通用工具栏：撤销/重做 =====
    QToolBar *editToolBar = addToolBar(tr("编辑"));
    editToolBar->setObjectName("editToolBar");
    editToolBar->setIconSize(QSize(22, 22));

    QAction *undoBtn = m_undoStack->createUndoAction(this);
    undoBtn->setIcon(QIcon(":/icons/undo.svg"));
    undoBtn->setToolTip(tr("撤销 (Ctrl+Z)"));
    editToolBar->addAction(undoBtn);

    QAction *redoBtn = m_undoStack->createRedoAction(this);
    redoBtn->setIcon(QIcon(":/icons/redo.svg"));
    redoBtn->setToolTip(tr("重做 (Ctrl+Y)"));
    editToolBar->addAction(redoBtn);

    editToolBar->addSeparator();

    // ===== 绘图工具栏 =====
    QToolBar *drawToolBar = addToolBar(tr("绘图工具"));
    drawToolBar->setObjectName("drawToolBar");
    drawToolBar->setIconSize(QSize(28, 28));

    drawToolBar->addAction(m_toolManager->createToolAction(0, this)); // 选择
    drawToolBar->addSeparator();
    drawToolBar->addAction(m_toolManager->createToolAction(1, this)); // 矩形
    drawToolBar->addAction(m_toolManager->createToolAction(2, this)); // 椭圆
    drawToolBar->addAction(m_toolManager->createToolAction(3, this)); // 线段
    drawToolBar->addAction(m_toolManager->createToolAction(4, this)); // 画笔
    drawToolBar->addAction(m_toolManager->createToolAction(5, this)); // 文字
    drawToolBar->addAction(m_toolManager->createToolAction(6, this)); // 橡皮擦

    // 快捷键绑定
    struct { QString key; QString name; } binds[] = {
        {"V", "选择"}, {"R", "矩形"}, {"E", "椭圆"},
        {"L", "线段"}, {"P", "画笔"}, {"T", "文字"}, {"X", "橡皮擦"}
    };
    for (auto &b : binds) {
        connect(new QShortcut(QKeySequence(b.key), this), &QShortcut::activated,
                this, [this, b]() { m_toolManager->setActiveTool(b.name); });
    }

    addToolBarBreak();

    // ===== 文字格式工具栏（仅文字选中时显示） =====
    m_textToolBar = addToolBar(tr("文字格式"));
    m_textToolBar->setObjectName("textToolBar");
    m_textToolBar->setVisible(false);

    QFontComboBox *fontCombo = new QFontComboBox(this);
    fontCombo->setCurrentFont(QFont("Microsoft YaHei"));
    fontCombo->setToolTip(tr("字体"));
    fontCombo->setMaximumWidth(160);
    m_textToolBar->addWidget(fontCombo);

    QSpinBox *fontSizeSpin = new QSpinBox(this);
    fontSizeSpin->setRange(8, 200);
    fontSizeSpin->setValue(16);
    fontSizeSpin->setToolTip(tr("字号"));
    fontSizeSpin->setMaximumWidth(60);
    m_textToolBar->addWidget(fontSizeSpin);

    // 字体变更 → 更新选中文字
    auto applyFont = [this, fontCombo, fontSizeSpin]() {
        QList<QGraphicsItem*> sel = m_canvasScene->selectedItems();
        for (auto *item : sel) {
            TextShape *text = dynamic_cast<TextShape*>(item);
            if (text) {
                QFont f = text->font();
                f.setFamily(fontCombo->currentFont().family());
                f.setPointSize(fontSizeSpin->value());
                text->setFont(f);
                m_canvasScene->setModified(true);
            }
        }
        // 同步更新 TextTool 的默认字体
        TextTool *tt = dynamic_cast<TextTool*>(m_toolManager->tool("文字"));
        if (tt) tt->setDefaultFont(QFont(fontCombo->currentFont().family(),
                                          fontSizeSpin->value()));
    };
    connect(fontCombo, &QFontComboBox::currentFontChanged, this, applyFont);
    connect(fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, applyFont);
}

void MainWindow::setupDockWidgets()
{
    // 图层面板（左侧）
    m_layerPanel = new LayerPanel(this);
    m_layerPanel->setDocument(m_document);
    m_layerDock = new QDockWidget(tr("图层"), this);
    m_layerDock->setObjectName("layerDock");
    m_layerDock->setWidget(m_layerPanel);
    m_layerDock->setMinimumWidth(180);
    addDockWidget(Qt::LeftDockWidgetArea, m_layerDock);

    // 属性面板（右侧）
    m_propertyPanel = new PropertyPanel(this);
    m_propertyPanel->setScene(m_canvasScene);
    m_propertyDock = new QDockWidget(tr("属性"), this);
    m_propertyDock->setObjectName("propertyDock");
    m_propertyDock->setWidget(m_propertyPanel);
    m_propertyDock->setMinimumWidth(200);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel(tr("就绪"));
    m_layerLabel  = new QLabel(tr("图层: 1"));

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_layerLabel);

    // ===== 缩放控件： [-] 百分比 [+] =====
    QWidget *zoomWidget = new QWidget(this);
    QHBoxLayout *zoomLayout = new QHBoxLayout(zoomWidget);
    zoomLayout->setContentsMargins(0, 0, 0, 0);
    zoomLayout->setSpacing(2);

    QPushButton *zoomOutBtn = new QPushButton("-");
    zoomOutBtn->setFixedSize(24, 24);
    zoomOutBtn->setToolTip(tr("缩小 (25%)"));

    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_zoomLabel->setMinimumWidth(48);

    QPushButton *zoomInBtn = new QPushButton("+");
    zoomInBtn->setFixedSize(24, 24);
    zoomInBtn->setToolTip(tr("放大 (25%)"));

    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(m_zoomLabel);
    zoomLayout->addWidget(zoomInBtn);

    connect(zoomOutBtn, &QPushButton::clicked, m_canvasView, [this]() {
        m_canvasView->setZoomLevel(m_canvasView->zoomLevel() - 0.25);
    });
    connect(zoomInBtn, &QPushButton::clicked, m_canvasView, [this]() {
        m_canvasView->setZoomLevel(m_canvasView->zoomLevel() + 0.25);
    });

    statusBar()->addPermanentWidget(zoomWidget);
}

void MainWindow::setupConnections()
{
    // 工具切换 → 更新光标
    connect(m_toolManager, &ToolManager::activeToolChanged, this, [this](ToolBase *tool) {
        if (tool && m_canvasView) {
            m_canvasView->viewport()->setCursor(tool->cursor());
        }
    });

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
        m_modified = !clean;
        updateWindowTitle();
    });

    connect(m_canvasView, &CanvasView::zoomChanged, this, [this](qreal zoom) {
        m_zoomLabel->setText(tr("%1%").arg(qRound(zoom * 100)));
    });

    connect(m_canvasScene, &CanvasScene::itemSelected, this, [this](QGraphicsItem *item) {
        m_propertyPanel->onSelectionChanged();
        updateStatusBar();
        // 文字选中时显示文字工具栏
        m_textToolBar->setVisible(dynamic_cast<TextShape*>(item) != nullptr);
    });
    connect(m_canvasScene, &CanvasScene::itemDeselected, this, [this]() {
        m_propertyPanel->clearSelection();
        m_textToolBar->setVisible(false);
        updateStatusBar();
    });

    connect(m_canvasScene, &CanvasScene::sceneModified, this, [this]() {
        if (!m_modified) { m_modified = true; updateWindowTitle(); }
        // 图层同步
        QList<QGraphicsItem*> allItems = m_canvasScene->items();
        Layer *layer = m_document->activeLayer();
        if (layer) {
            for (auto *item : allItems) {
                ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
                if (shape && !shape->parentItem() && !layer->shapes().contains(shape))
                    layer->addShape(shape);
            }
        }
    });

    connect(m_document, &Document::modified, this, [this]() {
        if (!m_modified) { m_modified = true; updateWindowTitle(); }
    });
}

void MainWindow::updateStatusBar()
{
    int selCount = m_canvasScene->selectedItems().size();
    int totalItems = m_canvasScene->items().size();
    m_statusLabel->setText(selCount > 0 ? tr("选中: %1 个图形").arg(selCount) : tr("就绪"));
    m_layerLabel->setText(tr("图层: %1 | 图形: %2")
        .arg(m_document->layerCount()).arg(totalItems));
}

void MainWindow::updateWindowTitle()
{
    QString title = tr("Paint - 轻量级矢量画板");
    if (!m_currentFilePath.isEmpty())
        title = QFileInfo(m_currentFilePath).fileName() + " - " + title;
    if (m_modified) title = "* " + title;
    setWindowTitle(title);
}

bool MainWindow::maybeSave()
{
    if (!m_modified) return true;
    auto ret = QMessageBox::warning(this, tr("未保存的更改"),
        tr("文档已被修改。是否保存更改？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save: onSaveFile(); return !m_modified;
    case QMessageBox::Discard: return true;
    default: return false;
    }
}

void MainWindow::onNewFile()
{
    if (!maybeSave()) return;
    m_currentFilePath.clear();
    m_undoStack->clear();
    m_canvasScene->blockSignals(true);
    m_canvasScene->clear();
    m_canvasScene->blockSignals(false);
    delete m_document;
    m_document = new Document(this);
    m_document->applyToScene(m_canvasScene);
    m_layerPanel->setDocument(m_document);
    m_modified = false;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onOpenFile()
{
    if (!maybeSave()) return;
    QString path = QFileDialog::getOpenFileName(this,
        tr("打开文件"), QString(), tr("矢量画板文件 (*.vdraw);;所有文件 (*)"));
    if (path.isEmpty()) return;
    Document *doc = m_fileManager->loadDocument(path);
    if (!doc) {
        QMessageBox::warning(this, tr("打开失败"), tr("无法打开文件：%1").arg(path));
        return;
    }
    m_canvasScene->blockSignals(true);
    m_canvasScene->clear();
    m_canvasScene->blockSignals(false);
    delete m_document;
    m_document = doc;
    m_document->applyToScene(m_canvasScene);
    m_layerPanel->setDocument(m_document);
    m_currentFilePath = path;
    m_undoStack->clear();
    m_modified = false;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onSaveFile()
{
    if (m_currentFilePath.isEmpty()) { onSaveAsFile(); return; }
    if (m_fileManager->saveDocument(m_currentFilePath, m_document)) {
        m_modified = false; m_undoStack->setClean(); updateWindowTitle();
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("无法保存文件：%1").arg(m_currentFilePath));
    }
}

void MainWindow::onSaveAsFile()
{
    QString path = QFileDialog::getSaveFileName(this,
        tr("另存为"), QString(), tr("矢量画板文件 (*.vdraw);;所有文件 (*)"));
    if (path.isEmpty()) return;
    if (!path.endsWith(".vdraw", Qt::CaseInsensitive)) path += ".vdraw";
    if (m_fileManager->saveDocument(path, m_document)) {
        m_currentFilePath = path; m_modified = false; m_undoStack->setClean();
        updateWindowTitle();
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("无法保存文件：%1").arg(path));
    }
}

void MainWindow::onExportSvg()
{
    QString path = QFileDialog::getSaveFileName(this, tr("导出 SVG"), QString(), tr("SVG 文件 (*.svg)"));
    if (path.isEmpty()) return;
    if (!m_fileManager->exportSvg(path, m_canvasScene))
        QMessageBox::warning(this, tr("导出失败"), tr("无法导出 SVG：%1").arg(path));
}

void MainWindow::onExportPng()
{
    QString path = QFileDialog::getSaveFileName(this, tr("导出 PNG"), QString(), tr("PNG 图片 (*.png)"));
    if (path.isEmpty()) return;
    if (!m_fileManager->exportPng(path, m_canvasScene))
        QMessageBox::warning(this, tr("导出失败"), tr("无法导出 PNG：%1").arg(path));
}

void MainWindow::onDeleteSelected()
{
    QList<QGraphicsItem*> selected = m_canvasScene->selectedItems();
    if (selected.isEmpty()) return;
    m_undoStack->beginMacro(tr("删除图形"));
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (shape)
            m_undoStack->push(new RemoveShapeCommand(shape, m_canvasScene, m_document->activeLayer()));
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

void MainWindow::onSelectAll()
{
    for (auto *item : m_canvasScene->items()) item->setSelected(true);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) event->accept();
    else event->ignore();
}
