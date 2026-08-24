#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QDockWidget>
#include <QMenu>
#include <QUndoStack>
#include <QApplication>
#include <QShortcut>
#include <QActionGroup>
#include <QPushButton>
#include <QComboBox>
#include <QToolButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QClipboard>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "canvas/CanvasView.h"
#include "canvas/CanvasScene.h"
#include "tools/ToolManager.h"
#include "tools/SelectTool.h"
#include "tools/DirectSelectTool.h"
#include "tools/ShapeTool.h"
#include "tools/FreehandTool.h"
#include "tools/TextTool.h"
#include "tools/EraserTool.h"
#include "tools/RulerTool.h"
#include "document/Document.h"
#include "layers/Layer.h"
#include "layers/LayerPanel.h"
#include "properties/PropertyPanel.h"
#include "history/HistoryPanel.h"
#include "shapes/ShapeBase.h"
#include "shapes/TextShape.h"
#include "document/FileManager.h"
#include "commands/RemoveShapeCommand.h"
#include "commands/AddShapeCommand.h"
#include "commands/TransformCommand.h"
#include "commands/FontCommand.h"
#include "commands/LayerCommand.h"
#include "commands/GroupCommand.h"
#include "commands/ReorderShapesCommand.h"
#include "commands/GroupRenameCommand.h"

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

    // 注册工具: 0=移动 1=直接选择 2=图形 3=自由钢笔 4=文字 5=橡皮擦 6=尺子
    m_toolManager->registerTool(new SelectTool(this));
    m_toolManager->registerTool(new DirectSelectTool(this));
    m_toolManager->registerTool(new ShapeTool(this));
    m_toolManager->registerTool(new FreehandTool(this));
    m_toolManager->registerTool(new TextTool(this));
    m_toolManager->registerTool(new EraserTool(this));
    m_toolManager->registerTool(new RulerTool(this));
    m_toolManager->setActiveTool(0);

    // 创建文档和文件管理器
    m_document = new Document(this);
    m_canvasScene->setDocument(m_document);
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

void MainWindow::setupMenuBar()
{
    // 禁用原生菜单栏，防止 Windows 吞噬 Alt 键（图形工具需要 Alt 用于中心绘制）
    menuBar()->setNativeMenuBar(false);

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
    QAction *cutAction = editMenu->addAction(tr("剪切(&T)"));
    cutAction->setShortcut(QKeySequence::Cut);
    connect(cutAction, &QAction::triggered, this, &MainWindow::cutSelection);
    QAction *copyAction = editMenu->addAction(tr("复制(&C)"));
    copyAction->setShortcut(QKeySequence::Copy);
    connect(copyAction, &QAction::triggered, this, &MainWindow::copySelection);
    QAction *pasteAction = editMenu->addAction(tr("粘贴(&P)"));
    pasteAction->setShortcut(QKeySequence::Paste);
    connect(pasteAction, &QAction::triggered, this, &MainWindow::pasteClipboard);
    QAction *duplicateAction = editMenu->addAction(tr("重复(&D)"));
    duplicateAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
    connect(duplicateAction, &QAction::triggered, this, &MainWindow::duplicateSelection);
    editMenu->addSeparator();
    QAction *deleteAction = editMenu->addAction(tr("删除(&Del)"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    QAction *selectAllAction = editMenu->addAction(tr("全选(&A)"));
    selectAllAction->setShortcut(QKeySequence::SelectAll);
    connect(selectAllAction, &QAction::triggered, this, &MainWindow::onSelectAll);

    // 对象菜单：编组/层级/对齐
    QMenu *objectMenu = menuBar()->addMenu(tr("对象(&O)"));
    QAction *groupAction = objectMenu->addAction(tr("编组"));
    groupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    connect(groupAction, &QAction::triggered, this, &MainWindow::groupSelected);
    QAction *ungroupAction = objectMenu->addAction(tr("解组"));
    ungroupAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
    connect(ungroupAction, &QAction::triggered, this, &MainWindow::ungroupSelected);
    objectMenu->addSeparator();
    QAction *frontAction = objectMenu->addAction(tr("置于顶层"));
    frontAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketRight));
    connect(frontAction, &QAction::triggered, this, &MainWindow::bringToFront);
    QAction *forwardAction = objectMenu->addAction(tr("上移一层"));
    forwardAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    connect(forwardAction, &QAction::triggered, this, &MainWindow::bringForward);
    QAction *backwardAction = objectMenu->addAction(tr("下移一层"));
    backwardAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    connect(backwardAction, &QAction::triggered, this, &MainWindow::sendBackward);
    QAction *backAction = objectMenu->addAction(tr("置于底层"));
    backAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_BracketLeft));
    connect(backAction, &QAction::triggered, this, &MainWindow::sendToBack);

    objectMenu->addSeparator();
    QMenu *alignMenu = objectMenu->addMenu(tr("对齐"));
    auto addAlign = [&](const QString &label, MainWindow::AlignMode mode) {
        QAction *a = alignMenu->addAction(label);
        connect(a, &QAction::triggered, this, [this, mode]() { alignSelected(mode); });
    };
    addAlign(tr("左对齐"),   MainWindow::AlignLeft);
    addAlign(tr("水平居中"), MainWindow::AlignHCenter);
    addAlign(tr("右对齐"),   MainWindow::AlignRight);
    addAlign(tr("顶对齐"),   MainWindow::AlignTop);
    addAlign(tr("垂直居中"), MainWindow::AlignVCenter);
    addAlign(tr("底对齐"),   MainWindow::AlignBottom);

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
    viewMenu->addSeparator();
    QAction *rulersAction = viewMenu->addAction(tr("显示标尺"));
    rulersAction->setCheckable(true);
    rulersAction->setChecked(m_canvasView->rulersVisible());
    rulersAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
    connect(rulersAction, &QAction::toggled, m_canvasView, &CanvasView::setRulersVisible);
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
    QAction *shortcutsAction = helpMenu->addAction(tr("快捷键(&K)"));
    connect(shortcutsAction, &QAction::triggered, this, [this]() {
        const QString text =
            QStringLiteral("【%1】\n").arg(tr("绘图工具"))
            + "V — 移动\nA — 直接选择\nS — 图形\nP — 自由钢笔\nT — 文字\nX — 橡皮擦\nR — 尺子\n\n"
            + QStringLiteral("【%1】\n").arg(tr("尺子工具"))
            + "O — 切换横/竖\nH — 显示/隐藏\n\n"
            + QStringLiteral("【%1】\n").arg(tr("文件"))
            + "Ctrl+N — 新建\nCtrl+O — 打开\nCtrl+S — 保存\nCtrl+Shift+S — 另存为\nCtrl+Q — 退出\n\n"
            + QStringLiteral("【%1】\n").arg(tr("编辑"))
            + "Ctrl+Z — 撤销\nCtrl+Y — 重做\nCtrl+X — 剪切\nCtrl+C — 复制\nCtrl+V — 粘贴\n"
              "Ctrl+D — 重复\nDelete — 删除\nCtrl+A — 全选\n\n"
            + QStringLiteral("【%1】\n").arg(tr("对象"))
            + "Ctrl+G — 编组\nCtrl+Shift+G — 解组\nCtrl+Shift+] — 置于顶层\nCtrl+] — 上移一层\n"
              "Ctrl+[ — 下移一层\nCtrl+Shift+[ — 置于底层\n\n"
            + QStringLiteral("【%1】\n").arg(tr("视图"))
            + "Ctrl+= — 放大\nCtrl+- — 缩小\nCtrl+0 — 适合窗口\nCtrl+Shift+R — 显示标尺\n\n"
            + QStringLiteral("【%1】\n").arg(tr("画布"))
            + "方向键 — 微调(1px)\nShift+方向键 — 微调(10px)\nCtrl+滚轮 — 缩放\n中键/Alt+左键拖拽 — 平移";
        QMessageBox::information(this, tr("快捷键"), text);
    });
    QAction *aboutAction = helpMenu->addAction(tr("关于(&A)"));
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, tr("关于 Paint"),
            tr("<h3>Paint 轻量级矢量画板</h3>"
               "<p>版本 1.0.0</p><p>基于 Qt6 的轻量级矢量图形绘制工具。</p>"));
    });
}

void MainWindow::setupToolBars()
{
    setupToolBarEdit();
    setupToolBarDraw();
    addToolBarBreak();
    setupToolBarShape();
    addToolBarBreak();
    setupToolBarText();
}

void MainWindow::setupToolBarEdit()
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
}

void MainWindow::setupToolBarDraw()
{
    // ===== 绘图工具栏 =====
    QToolBar *drawToolBar = addToolBar(tr("绘图工具"));
    drawToolBar->setObjectName("drawToolBar");
    drawToolBar->setIconSize(QSize(28, 28));

    drawToolBar->addAction(m_toolManager->createToolAction(0, this)); // 移动
    drawToolBar->addAction(m_toolManager->createToolAction(1, this)); // 直接选择
    drawToolBar->addSeparator();
    drawToolBar->addAction(m_toolManager->createToolAction(2, this)); // 图形
    drawToolBar->addAction(m_toolManager->createToolAction(3, this)); // 自由钢笔
    drawToolBar->addAction(m_toolManager->createToolAction(4, this)); // 文字
    drawToolBar->addAction(m_toolManager->createToolAction(5, this)); // 橡皮擦
    drawToolBar->addSeparator();
    drawToolBar->addAction(m_toolManager->createToolAction(6, this)); // 尺子

    // 尺子方向切换按钮（横/竖）
    m_rulerOrientationBtn = new QToolButton(this);
    m_rulerOrientationBtn->setText(tr("横"));
    m_rulerOrientationBtn->setToolTip(tr("切换尺子方向（横/竖）"));
    m_rulerOrientationBtn->setCheckable(true);
    connect(m_rulerOrientationBtn, &QToolButton::clicked,
            m_canvasScene, &CanvasScene::toggleRulerOrientation);
    connect(m_canvasScene, &CanvasScene::rulerChanged, this, [this]() {
        bool horiz = (m_canvasScene->ruler().orientation == Qt::Horizontal);
        m_rulerOrientationBtn->setText(horiz ? tr("横") : tr("竖"));
        m_rulerOrientationBtn->setChecked(horiz);
    });
    m_rulerOrientationBtn->setChecked(true); // 默认水平
    drawToolBar->addWidget(m_rulerOrientationBtn);

    drawToolBar->addSeparator();

    // ===== 通用描边控件（图形工具 & 画笔工具共享） =====

    // 描边颜色按钮
    m_strokeColorBtn = new QPushButton(this);
    m_strokeColorBtn->setToolTip(tr("描边颜色"));
    m_strokeColorBtn->setFixedSize(28, 28);
    auto updateStrokeColorBtn = [this](const QColor &color) {
        m_strokeColorBtn->setStyleSheet(
            QString("QPushButton { background-color: %1; }").arg(color.name()));
    };
    updateStrokeColorBtn(Qt::black);
    drawToolBar->addWidget(m_strokeColorBtn);

    // 描边样式下拉
    m_strokeStyleCombo = new QComboBox(this);
    m_strokeStyleCombo->setToolTip(tr("描边样式"));
    m_strokeStyleCombo->addItem(tr("─ 实线"),     static_cast<int>(Qt::SolidLine));
    m_strokeStyleCombo->addItem(tr("- - 线段虚线"), static_cast<int>(Qt::DashLine));
    m_strokeStyleCombo->addItem(tr("· · 点虚线"),  static_cast<int>(Qt::DotLine));
    m_strokeStyleCombo->setMaximumWidth(110);
    drawToolBar->addWidget(m_strokeStyleCombo);

    // 描边粗细
    m_strokeWidthSpin = new QDoubleSpinBox(this);
    m_strokeWidthSpin->setRange(0.5, 50.0);
    m_strokeWidthSpin->setValue(2.0);
    m_strokeWidthSpin->setSingleStep(0.5);
    m_strokeWidthSpin->setDecimals(1);
    m_strokeWidthSpin->setToolTip(tr("描边宽度"));
    m_strokeWidthSpin->setMaximumWidth(100);
    m_strokeWidthSpin->setSuffix(tr("px"));
    drawToolBar->addWidget(m_strokeWidthSpin);

    // 快捷键绑定
    struct { QString key; QString name; } binds[] = {
        {"V", "移动"}, {"A", "直接选择"}, {"S", "图形"},
        {"P", "自由钢笔"}, {"T", "文字"}, {"X", "橡皮擦"},
        {"R", "尺子"}
    };
    for (auto &b : binds) {
        connect(new QShortcut(QKeySequence(b.key), this), &QShortcut::activated,
                this, [this, b]() { m_toolManager->setActiveTool(b.name); });
    }

    // ═══════════════════════════════════════
    // 描边控件 ↔ 活动工具（通过虚接口，无需 dynamic_cast）
    // ═══════════════════════════════════════

    // 描边色按钮点击 → 颜色对话框
    connect(m_strokeColorBtn, &QPushButton::clicked, this, [this, updateStrokeColorBtn]() {
        ToolBase *tool = m_toolManager->activeTool();
        if (!tool || !tool->supportsStroke()) return;
        QColor color = QColorDialog::getColor(tool->strokeColor(), this, tr("选择描边色"));
        if (color.isValid()) {
            tool->setStrokeColor(color);
            updateStrokeColorBtn(color);
        }
    });

    // 描边样式下拉
    connect(m_strokeStyleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int /*idx*/) {
        ToolBase *tool = m_toolManager->activeTool();
        if (tool && tool->supportsStroke())
            tool->setStrokeStyle(m_strokeStyleCombo->currentData().toInt());
    });

    // 描边粗细
    connect(m_strokeWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this](double val) {
        ToolBase *tool = m_toolManager->activeTool();
        if (tool && tool->supportsStroke())
            tool->setStrokeWidth(val);
    });

    // 切换工具时，将控件同步到新工具的当前值
    connect(m_toolManager, &ToolManager::activeToolChanged, this,
            [this, updateStrokeColorBtn](ToolBase *tool) {
        bool supports = tool && tool->supportsStroke();
        QColor c = supports ? tool->strokeColor() : Qt::black;
        qreal  w = supports ? tool->strokeWidth() : 2.0;
        int    s = supports ? tool->strokeStyle() : static_cast<int>(Qt::SolidLine);
        updateStrokeColorBtn(c);
        m_strokeWidthSpin->blockSignals(true);
        m_strokeWidthSpin->setValue(w);
        m_strokeWidthSpin->blockSignals(false);
        m_strokeStyleCombo->blockSignals(true);
        int idx = m_strokeStyleCombo->findData(s);
        if (idx >= 0) m_strokeStyleCombo->setCurrentIndex(idx);
        m_strokeStyleCombo->blockSignals(false);
    });
}

void MainWindow::setupToolBarShape()
{
    // ===== 图形子工具栏（仅 ShapeTool 激活时显示） =====
    m_shapeToolBar = addToolBar(tr("图形工具"));
    m_shapeToolBar->setObjectName("shapeToolBar");
    m_shapeToolBar->setVisible(false);

    // 图形类型按钮（互斥选择）
    QActionGroup *shapeGroup = new QActionGroup(this);
    shapeGroup->setExclusive(true);

    ShapeTool::ShapeType shapeTypes[] = {
        ShapeTool::Rect, ShapeTool::Ellipse, ShapeTool::Line,
        ShapeTool::Triangle, ShapeTool::Diamond, ShapeTool::Arrow
    };
    QString shapeTips[] = {
        tr("矩形"), tr("椭圆"), tr("线段"),
        tr("三角形"), tr("菱形"), tr("箭头")
    };

    for (int i = 0; i < 6; ++i) {
        QAction *act = new QAction(ShapeTool::iconForShape(shapeTypes[i]), shapeTips[i], this);
        act->setCheckable(true);
        act->setData(static_cast<int>(shapeTypes[i]));
        shapeGroup->addAction(act);
        m_shapeToolBar->addAction(act);
    }
    shapeGroup->actions().first()->setChecked(true);

    m_shapeToolBar->addSeparator();

    // 填充颜色按钮（仅图形工具）
    QPushButton *fillColorBtn = new QPushButton(tr("无填充"), this);
    fillColorBtn->setToolTip(tr("填充颜色"));
    fillColorBtn->setMinimumWidth(52);
    fillColorBtn->setFixedHeight(28);
    fillColorBtn->setStyleSheet("QPushButton { background-color: transparent; }");
    m_shapeToolBar->addWidget(fillColorBtn);

    // 清除填充按钮
    QPushButton *clearFillBtn = new QPushButton(tr("✕"), this);
    clearFillBtn->setToolTip(tr("清除填充"));
    clearFillBtn->setFixedSize(20, 28);
    clearFillBtn->setStyleSheet("QPushButton { font-size: 11px; font-weight: bold; }");
    m_shapeToolBar->addWidget(clearFillBtn);

    // ═══════════════════════════════════════
    // 图形子工具栏 ↔ ShapeTool
    // ═══════════════════════════════════════

    ShapeTool *shapeTool = dynamic_cast<ShapeTool*>(m_toolManager->tool("图形"));
    if (shapeTool) {
        // 图形类型按钮 → tool
        connect(shapeGroup, &QActionGroup::triggered, this, [shapeTool](QAction *action) {
            shapeTool->setShapeType(static_cast<ShapeTool::ShapeType>(action->data().toInt()));
        });

        // 填充色按钮 → 颜色对话框
        auto updateFillBtn = [fillColorBtn](const QColor &color) {
            if (!color.isValid() || color.alpha() == 0) {
                fillColorBtn->setText(tr("无填充"));
                fillColorBtn->setStyleSheet("QPushButton { background-color: transparent; }");
            } else {
                fillColorBtn->setText("");
                fillColorBtn->setStyleSheet(
                    QString("QPushButton { background-color: %1; }")
                        .arg(color.name(QColor::HexArgb)));
            }
        };

        connect(fillColorBtn, &QPushButton::clicked, this, [this, shapeTool, updateFillBtn]() {
            QColor initial = shapeTool->fillColor();
            QColor color = QColorDialog::getColor(
                initial.alpha() == 0 ? Qt::white : initial,
                this, tr("选择填充色"),
                QColorDialog::ShowAlphaChannel);
            if (color.isValid()) {
                shapeTool->setFillColor(color);
                updateFillBtn(color);
            }
        });

        // 清除填充按钮
        connect(clearFillBtn, &QPushButton::clicked, this, [shapeTool, updateFillBtn]() {
            shapeTool->setFillColor(Qt::transparent);
            updateFillBtn(Qt::transparent);
        });

        // tool → 子工具栏同步
        connect(shapeTool, &ShapeTool::shapeTypeChanged, this,
                [shapeGroup](ShapeTool::ShapeType type) {
            for (QAction *act : shapeGroup->actions()) {
                if (static_cast<ShapeTool::ShapeType>(act->data().toInt()) == type) {
                    act->setChecked(true);
                    break;
                }
            }
        });

        connect(shapeTool, &ShapeTool::fillColorChanged, this, updateFillBtn);
    }
}

void MainWindow::setupToolBarText()
{
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
        QString family = fontCombo->currentFont().family();
        int size = fontSizeSpin->value();

        QList<QGraphicsItem*> sel = m_canvasScene->selectedItems();
        if (!sel.isEmpty()) {
            m_canvasScene->beginUndoMacro(tr("更改字体"));
            for (auto *item : sel) {
                TextShape *text = dynamic_cast<TextShape*>(item);
                if (!text) continue;
                QFont newFont = text->font();
                newFont.setFamily(family);
                newFont.setPointSize(size);
                if (text->font() != newFont)
                    m_canvasScene->pushUndoCommand(new FontCommand(text, text->font(), newFont));
            }
            m_canvasScene->endUndoMacro();
            m_canvasScene->setModified(true);
        }
        // 同步更新 TextTool 的默认字体
        TextTool *tt = dynamic_cast<TextTool*>(m_toolManager->tool("文字"));
        if (tt) tt->setDefaultFont(QFont(family, size));
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
    addDockWidget(Qt::LeftDockWidgetArea, m_layerDock);
    resizeDocks({m_layerDock}, {120}, Qt::Horizontal);

    // 属性面板（右侧）
    m_propertyPanel = new PropertyPanel(this);
    m_propertyPanel->setScene(m_canvasScene);
    m_propertyDock = new QDockWidget(tr("属性"), this);
    m_propertyDock->setObjectName("propertyDock");
    m_propertyDock->setWidget(m_propertyPanel);
    m_propertyDock->setMinimumWidth(200);
    addDockWidget(Qt::RightDockWidgetArea, m_propertyDock);

    // 历史记录面板（右侧，属性下方）
    m_historyPanel = new HistoryPanel(this);
    m_historyPanel->setUndoStack(m_undoStack);
    m_historyDock = new QDockWidget(tr("历史记录"), this);
    m_historyDock->setObjectName("historyDock");
    m_historyDock->setWidget(m_historyPanel);
    addDockWidget(Qt::RightDockWidgetArea, m_historyDock);
    resizeDocks({m_historyDock}, {200}, Qt::Horizontal);
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

    QPushButton *zoomOutBtn = new QPushButton();
    zoomOutBtn->setIcon(QIcon(":/icons/zoom-out.svg"));
    zoomOutBtn->setIconSize(QSize(16, 16));
    zoomOutBtn->setFixedSize(24, 24);
    zoomOutBtn->setToolTip(tr("缩小 (25%)"));

    m_zoomLabel = new QLabel("100%");
    m_zoomLabel->setAlignment(Qt::AlignCenter);
    m_zoomLabel->setMinimumWidth(48);

    QPushButton *zoomInBtn = new QPushButton();
    zoomInBtn->setIcon(QIcon(":/icons/zoom-in.svg"));
    zoomInBtn->setIconSize(QSize(16, 16));
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
    // 工具状态消息 → 状态栏
    connect(m_toolManager, &ToolManager::statusMessage, this, [this](const QString &msg) {
        m_statusLabel->setText(msg);
    });

    // 工具切换 → 更新光标 & 文字工具栏
    connect(m_toolManager, &ToolManager::activeToolChanged, this, [this](ToolBase *tool) {
        if (tool && m_canvasView) {
            m_canvasView->viewport()->setCursor(tool->cursor());
        }
        // 切换工具时显示/隐藏对应子工具栏
        m_textToolBar->setVisible(tool && tool->name() == "文字");
        m_shapeToolBar->setVisible(tool && tool->name() == "图形");
    });

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
        m_modified = !clean;
        updateWindowTitle();
    });

    connect(m_canvasView, &CanvasView::zoomChanged, this, [this](qreal zoom) {
        m_zoomLabel->setText(tr("%1%").arg(qRound(zoom * 100)));
    });

    connect(m_canvasView, &CanvasView::nudgeRequested, this, &MainWindow::nudgeSelected);
    connect(m_canvasView, &CanvasView::contextMenuRequested, this, &MainWindow::showContextMenu);

    connect(m_canvasScene, &CanvasScene::itemSelected, this, [this](QGraphicsItem *item) {
        m_propertyPanel->onSelectionChanged();
        updateStatusBar();
        // 文字选中时显示文字工具栏
        m_textToolBar->setVisible(dynamic_cast<TextShape*>(item) != nullptr);
    });
    connect(m_canvasScene, &CanvasScene::itemDeselected, this, [this]() {
        m_propertyPanel->clearSelection();
        // 文字工具激活时保持工具栏可见，否则隐藏
        ToolBase *active = m_toolManager ? m_toolManager->activeTool() : nullptr;
        m_textToolBar->setVisible(active && active->name() == "文字");
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
        m_layerPanel->refreshTree();
        updateStatusBar();
    });

    // 撤销/重做后刷新状态栏计数、按图层重排 z 顺序、刷新组面板
    connect(m_undoStack, &QUndoStack::indexChanged, this, [this]() {
        updateStatusBar();
        restackByLayers();
        if (m_layerPanel) m_layerPanel->refreshGroupTree();
    });

    connect(m_document, &Document::modified, this, [this]() {
        if (!m_modified) { m_modified = true; updateWindowTitle(); }
    });

    // 图层操作 → 撤销栈
    connect(m_layerPanel, &LayerPanel::addLayerRequested, this, &MainWindow::onAddLayerRequested);
    connect(m_layerPanel, &LayerPanel::removeLayerRequested, this, &MainWindow::onRemoveLayerRequested);
    connect(m_layerPanel, &LayerPanel::moveLayerRequested, this, &MainWindow::onMoveLayerRequested);
    connect(m_layerPanel, &LayerPanel::renameLayerRequested, this, &MainWindow::onRenameLayerRequested);
    connect(m_layerPanel, &LayerPanel::renameGroupRequested, this, &MainWindow::onRenameGroupRequested);
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
    m_canvasScene->setDocument(m_document);
    m_document->applyToScene(m_canvasScene);
    resetGroupIdCounter();
    restackByLayers();
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
    m_canvasScene->setDocument(m_document);
    m_document->applyToScene(m_canvasScene);
    resetGroupIdCounter();
    restackByLayers();
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

void MainWindow::clearCanvas()
{
    QList<ShapeBase*> shapes;
    for (auto *item : m_canvasScene->items()) {
        auto *s = dynamic_cast<ShapeBase*>(item);
        if (s && !s->parentItem()) shapes.append(s);
    }
    if (shapes.isEmpty()) return;

    if (QMessageBox::question(this, tr("清空画布"),
            tr("将删除画布上所有图形，是否继续？"),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    m_undoStack->beginMacro(tr("清空画布"));
    for (auto *shape : shapes) {
        Layer *layer = m_document->layerOf(shape);
        m_undoStack->push(new RemoveShapeCommand(shape, m_canvasScene, layer));
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

// ===== 剪贴板 / 重复 / 方向键微调 =====

QJsonArray MainWindow::serializeSelection() const
{
    QJsonArray arr;
    for (auto *item : m_canvasScene->selectedItems()) {
        if (auto *shape = dynamic_cast<ShapeBase*>(item))
            arr.append(shape->toJson());
    }
    return arr;
}

void MainWindow::insertShapes(const QJsonArray &arr, const QPointF &offset)
{
    if (arr.isEmpty()) return;
    Layer *layer = m_document->activeLayer();

    m_canvasScene->clearSelection();
    m_undoStack->beginMacro(tr("粘贴"));
    for (const QJsonValue &v : arr) {
        ShapeBase *shape = ShapeBase::createFromJson(v.toObject());
        if (!shape) continue;
        shape->moveBy(offset.x(), offset.y());
        m_undoStack->push(new AddShapeCommand(shape, m_canvasScene, layer));
        shape->setSelected(true);
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

void MainWindow::copySelection()
{
    QJsonArray arr = serializeSelection();
    if (arr.isEmpty()) return;
    QJsonDocument doc(arr);
    QApplication::clipboard()->setText(QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
}

void MainWindow::cutSelection()
{
    copySelection();
    onDeleteSelected();
}

void MainWindow::pasteClipboard()
{
    QJsonDocument doc = QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8());
    if (!doc.isArray()) return;
    insertShapes(doc.array());
}

void MainWindow::pasteInPlace()
{
    QJsonDocument doc = QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8());
    if (!doc.isArray()) return;
    insertShapes(doc.array(), QPointF(0, 0));
}

void MainWindow::pasteAt(const QPointF &pos)
{
    QJsonDocument doc = QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8());
    if (!doc.isArray()) return;
    QJsonArray arr = doc.array();
    if (arr.isEmpty()) return;

    // 反序列化并计算整体包围盒
    QList<ShapeBase*> shapes;
    QRectF bbox;
    for (const QJsonValue &v : arr) {
        ShapeBase *shape = ShapeBase::createFromJson(v.toObject());
        if (!shape) continue;
        shapes.append(shape);
        QRectF r = shape->sceneBoundingRect();
        bbox = bbox.isEmpty() ? r : bbox.united(r);
    }
    if (shapes.isEmpty()) return;

    QPointF offset = pos - bbox.topLeft();
    Layer *layer = m_document->activeLayer();

    m_canvasScene->clearSelection();
    m_undoStack->beginMacro(tr("粘贴"));
    for (auto *shape : shapes) {
        shape->moveBy(offset.x(), offset.y());
        m_undoStack->push(new AddShapeCommand(shape, m_canvasScene, layer));
        shape->setSelected(true);
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

void MainWindow::duplicateSelection()
{
    insertShapes(serializeSelection());
}

void MainWindow::nudgeSelected(qreal dx, qreal dy)
{
    QList<QGraphicsItem*> selected = m_canvasScene->selectedItems();
    if (selected.isEmpty()) return;

    m_undoStack->beginMacro(tr("微调"));
    for (auto *item : selected) {
        QPointF oldPos = item->pos();
        QTransform oldXf = item->transform();
        item->moveBy(dx, dy);
        m_undoStack->push(new TransformCommand(item, oldPos, oldXf,
                                               item->pos(), item->transform()));
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

// ===== 图层操作（撤销栈入口） =====

void MainWindow::onAddLayerRequested()
{
    int index = m_document->layerCount();
    m_undoStack->push(new LayerCommand(
        LayerCommand::AddLayer, m_document, m_canvasScene, nullptr, index, -1, QString()));
}

void MainWindow::onRemoveLayerRequested(int index)
{
    if (m_document->layerCount() <= 1) return;  // 至少保留一个图层
    if (index < 0 || index >= m_document->layerCount()) return;
    Layer *layer = m_document->layers().at(index);
    m_undoStack->push(new LayerCommand(
        LayerCommand::RemoveLayer, m_document, m_canvasScene, layer, index, -1, QString()));
}

void MainWindow::onMoveLayerRequested(int from, int to)
{
    m_undoStack->push(new LayerCommand(
        LayerCommand::MoveLayer, m_document, m_canvasScene, nullptr, from, to, QString()));
}

void MainWindow::onRenameLayerRequested(int index, const QString &newName)
{
    if (index < 0 || index >= m_document->layerCount()) return;
    Layer *layer = m_document->layers().at(index);
    if (!layer || layer->name() == newName) return;
    m_undoStack->push(new LayerCommand(
        LayerCommand::RenameLayer, m_document, m_canvasScene, layer, index, -1, newName));
}

// ===== 层级与对齐 =====

QList<ShapeBase*> MainWindow::selectedShapes() const
{
    QList<ShapeBase*> shapes;
    for (auto *item : m_canvasScene->selectedItems()) {
        auto *s = dynamic_cast<ShapeBase*>(item);
        if (s && !s->parentItem()) shapes.append(s);
    }
    return shapes;
}

void MainWindow::restackByLayers()
{
    // 按图层顺序（底部图层在前）+ 图层内图形顺序，重新分配 z 值
    int z = 0;
    for (auto *layer : m_document->layers()) {
        for (auto *shape : layer->shapes()) {
            shape->setZValue(z++);
        }
    }
}

void MainWindow::pushLayerReorder(const QList<Layer*> &layers,
                                  const QList<QList<ShapeBase*>> &newOrders,
                                  const QString &text)
{
    if (layers.isEmpty()) return;
    QList<QList<ShapeBase*>> oldOrders;
    for (auto *layer : layers) oldOrders.append(layer->shapes());
    m_undoStack->push(new ReorderShapesCommand(layers, oldOrders, newOrders, text));
}

void MainWindow::bringToFront()
{
    QList<ShapeBase*> sel = selectedShapes();
    if (sel.isEmpty()) return;
    QList<Layer*> layers;
    QList<QList<ShapeBase*>> newOrders;
    for (auto *layer : m_document->layers()) {
        QList<ShapeBase*> newOrder;
        for (auto *s : layer->shapes()) if (!sel.contains(s)) newOrder.append(s);
        for (auto *s : layer->shapes()) if (sel.contains(s)) newOrder.append(s);
        if (newOrder != layer->shapes()) { layers.append(layer); newOrders.append(newOrder); }
    }
    pushLayerReorder(layers, newOrders, tr("置于顶层"));
}

void MainWindow::sendToBack()
{
    QList<ShapeBase*> sel = selectedShapes();
    if (sel.isEmpty()) return;
    QList<Layer*> layers;
    QList<QList<ShapeBase*>> newOrders;
    for (auto *layer : m_document->layers()) {
        QList<ShapeBase*> newOrder;
        for (auto *s : layer->shapes()) if (sel.contains(s)) newOrder.append(s);
        for (auto *s : layer->shapes()) if (!sel.contains(s)) newOrder.append(s);
        if (newOrder != layer->shapes()) { layers.append(layer); newOrders.append(newOrder); }
    }
    pushLayerReorder(layers, newOrders, tr("置于底层"));
}

void MainWindow::bringForward()
{
    QList<ShapeBase*> sel = selectedShapes();
    if (sel.isEmpty()) return;
    QList<Layer*> layers;
    QList<QList<ShapeBase*>> newOrders;
    for (auto *layer : m_document->layers()) {
        QList<ShapeBase*> cur = layer->shapes();
        int topIdx = -1, bottomIdx = cur.size();
        for (int i = 0; i < cur.size(); ++i) {
            if (sel.contains(cur[i])) {
                if (i < bottomIdx) bottomIdx = i;
                if (i > topIdx) topIdx = i;
            }
        }
        if (topIdx < 0 || topIdx + 1 >= cur.size()) continue;  // 已最顶
        if (sel.contains(cur[topIdx + 1])) continue;
        // 把上方紧邻的非选中项移到选中块下方（整块上移）
        ShapeBase *above = cur.takeAt(topIdx + 1);
        cur.insert(bottomIdx, above);
        layers.append(layer);
        newOrders.append(cur);
    }
    pushLayerReorder(layers, newOrders, tr("上移一层"));
}

void MainWindow::sendBackward()
{
    QList<ShapeBase*> sel = selectedShapes();
    if (sel.isEmpty()) return;
    QList<Layer*> layers;
    QList<QList<ShapeBase*>> newOrders;
    for (auto *layer : m_document->layers()) {
        QList<ShapeBase*> cur = layer->shapes();
        int topIdx = -1, bottomIdx = cur.size();
        for (int i = 0; i < cur.size(); ++i) {
            if (sel.contains(cur[i])) {
                if (i < bottomIdx) bottomIdx = i;
                if (i > topIdx) topIdx = i;
            }
        }
        if (bottomIdx <= 0) continue;  // 已最底
        if (sel.contains(cur[bottomIdx - 1])) continue;
        // 把下方紧邻的非选中项移到选中块上方（整块下移）
        ShapeBase *below = cur.takeAt(bottomIdx - 1);
        cur.insert(topIdx, below);
        layers.append(layer);
        newOrders.append(cur);
    }
    pushLayerReorder(layers, newOrders, tr("下移一层"));
}

void MainWindow::showContextMenu(const QPointF &scenePos, const QPoint &globalPos)
{
    QGraphicsItem *item = m_canvasScene->itemAt(scenePos, QTransform());
    ShapeBase *shape = dynamic_cast<ShapeBase*>(item);

    if (!shape) {
        // 空白画布菜单
        QMenu menu(this);
        QAction *pasteAct = menu.addAction(tr("粘贴"));
        pasteAct->setEnabled(canPaste());
        connect(pasteAct, &QAction::triggered, this, [this, scenePos]() { pasteAt(scenePos); });
        QAction *pasteInPlaceAct = menu.addAction(tr("粘贴到原位置"));
        pasteInPlaceAct->setEnabled(canPaste());
        connect(pasteInPlaceAct, &QAction::triggered, this, &MainWindow::pasteInPlace);
        QAction *selectAllAct = menu.addAction(tr("全选"));
        connect(selectAllAct, &QAction::triggered, this, &MainWindow::onSelectAll);
        QAction *clearAct = menu.addAction(tr("清空画布"));
        connect(clearAct, &QAction::triggered, this, &MainWindow::clearCanvas);

        menu.addSeparator();
        QMenu *bgMenu = menu.addMenu(tr("画布背景"));
        auto addBg = [&](const QString &name, int type) {
            QAction *a = bgMenu->addAction(name);
            connect(a, &QAction::triggered, this, [this, type]() {
                m_canvasView->setBackgroundType(type);
            });
        };
        addBg(tr("纯白"), 0);
        addBg(tr("格子"), 1);
        addBg(tr("点阵"), 2);

        QAction *fitAct = menu.addAction(tr("适合窗口"));
        connect(fitAct, &QAction::triggered, m_canvasView, &CanvasView::zoomFit);

        menu.exec(globalPos);
        return;
    }

    // 图形菜单：右键的图形若未选中，则选中它（已有选中则保持多选）
    if (!shape->isSelected()) {
        m_canvasScene->clearSelection();
        shape->setSelected(true);
    }

    QMenu menu(this);
    QAction *copyAct = menu.addAction(tr("复制"));
    connect(copyAct, &QAction::triggered, this, &MainWindow::copySelection);
    QAction *deleteAct = menu.addAction(tr("删除"));
    connect(deleteAct, &QAction::triggered, this, &MainWindow::onDeleteSelected);

    menu.addSeparator();

    QAction *groupAct = menu.addAction(tr("编组"));
    connect(groupAct, &QAction::triggered, this, &MainWindow::groupSelected);
    QAction *ungroupAct = menu.addAction(tr("解组"));
    connect(ungroupAct, &QAction::triggered, this, &MainWindow::ungroupSelected);

    menu.addSeparator();

    QAction *frontAct = menu.addAction(tr("置于顶层"));
    connect(frontAct, &QAction::triggered, this, &MainWindow::bringToFront);
    QAction *forwardAct = menu.addAction(tr("上移一层"));
    connect(forwardAct, &QAction::triggered, this, &MainWindow::bringForward);
    QAction *backwardAct = menu.addAction(tr("下移一层"));
    connect(backwardAct, &QAction::triggered, this, &MainWindow::sendBackward);
    QAction *backAct = menu.addAction(tr("置于底层"));
    connect(backAct, &QAction::triggered, this, &MainWindow::sendToBack);

    menu.addSeparator();

    QMenu *alignMenu = menu.addMenu(tr("对齐"));
    auto addAlign = [&](const QString &label, MainWindow::AlignMode mode) {
        QAction *a = alignMenu->addAction(label);
        connect(a, &QAction::triggered, this, [this, mode]() { alignSelected(mode); });
    };
    addAlign(tr("左对齐"),   MainWindow::AlignLeft);
    addAlign(tr("水平居中"), MainWindow::AlignHCenter);
    addAlign(tr("右对齐"),   MainWindow::AlignRight);
    addAlign(tr("顶对齐"),   MainWindow::AlignTop);
    addAlign(tr("垂直居中"), MainWindow::AlignVCenter);
    addAlign(tr("底对齐"),   MainWindow::AlignBottom);

    menu.exec(globalPos);
}

bool MainWindow::canPaste() const
{
    QJsonDocument doc = QJsonDocument::fromJson(QApplication::clipboard()->text().toUtf8());
    return doc.isArray() && !doc.array().isEmpty();
}

void MainWindow::alignSelected(MainWindow::AlignMode mode)
{
    QList<ShapeBase*> shapes = selectedShapes();
    if (shapes.size() < 2) return;

    struct Box { ShapeBase *shape; QRectF rect; };
    QList<Box> boxes;
    QRectF all;
    for (auto *s : shapes) {
        QRectF r = s->mapToScene(s->contentRect()).boundingRect();
        boxes.append({s, r});
        all = all.isEmpty() ? r : all.united(r);
    }

    qreal target;
    switch (mode) {
    case AlignLeft:    target = all.left();       break;
    case AlignRight:   target = all.right();      break;
    case AlignHCenter: target = all.center().x(); break;
    case AlignTop:     target = all.top();        break;
    case AlignBottom:  target = all.bottom();     break;
    case AlignVCenter: target = all.center().y(); break;
    default: return;
    }

    m_undoStack->beginMacro(tr("对齐"));
    for (const Box &b : boxes) {
        QPointF delta;
        switch (mode) {
        case AlignLeft:    delta.setX(target - b.rect.left());       break;
        case AlignRight:   delta.setX(target - b.rect.right());      break;
        case AlignHCenter: delta.setX(target - b.rect.center().x()); break;
        case AlignTop:     delta.setY(target - b.rect.top());        break;
        case AlignBottom:  delta.setY(target - b.rect.bottom());     break;
        case AlignVCenter: delta.setY(target - b.rect.center().y()); break;
        default: break;
        }
        if (delta.isNull()) continue;
        QPointF oldPos = b.shape->pos();
        b.shape->setPos(oldPos + delta);
        m_undoStack->push(new TransformCommand(
            b.shape, oldPos, b.shape->transform(),
            b.shape->pos(), b.shape->transform()));
    }
    m_undoStack->endMacro();
    m_canvasScene->setModified(true);
}

// ===== 编组 / 解组 =====

void MainWindow::groupSelected()
{
    QList<ShapeBase*> shapes = selectedShapes();
    if (shapes.size() < 2) return;

    // 把已编组的同组图形一并纳入，去重
    QList<ShapeBase*> list;
    for (auto *s : shapes) {
        if (s->groupId() >= 0) {
            for (auto *item : m_canvasScene->items()) {
                auto *o = dynamic_cast<ShapeBase*>(item);
                if (o && o->groupId() == s->groupId() && !list.contains(o))
                    list.append(o);
            }
        } else if (!list.contains(s)) {
            list.append(s);
        }
    }
    if (list.size() < 2) return;

    QList<qint64> oldIds;
    for (auto *s : list) oldIds.append(s->groupId());
    qint64 newId = m_nextGroupId++;
    QList<qint64> newIds(list.size(), newId);

    QMap<qint64, QString> oldNames = m_document->groupNames();
    QMap<qint64, QString> newNames = oldNames;
    newNames.insert(newId, tr("组 %1").arg(newId));
    m_undoStack->push(new GroupCommand(m_document, list, oldIds, newIds,
                                       oldNames, newNames, tr("编组")));
    m_canvasScene->setModified(true);
}

void MainWindow::ungroupSelected()
{
    QList<qint64> groups;
    for (auto *s : selectedShapes())
        if (s->groupId() >= 0 && !groups.contains(s->groupId()))
            groups.append(s->groupId());
    if (groups.isEmpty()) return;

    QList<ShapeBase*> list;
    for (auto *item : m_canvasScene->items()) {
        auto *o = dynamic_cast<ShapeBase*>(item);
        if (o && groups.contains(o->groupId()))
            list.append(o);
    }
    if (list.isEmpty()) return;

    QList<qint64> oldIds;
    for (auto *s : list) oldIds.append(s->groupId());
    QList<qint64> newIds(list.size(), -1);

    QMap<qint64, QString> oldNames = m_document->groupNames();
    QMap<qint64, QString> newNames = oldNames;
    for (qint64 gid : groups)
        newNames.remove(gid);
    m_undoStack->push(new GroupCommand(m_document, list, oldIds, newIds,
                                       oldNames, newNames, tr("解组")));
    m_canvasScene->setModified(true);
}

void MainWindow::onRenameGroupRequested(qint64 groupId, const QString &newName)
{
    QString oldName = m_document->groupName(groupId);
    if (oldName == newName) return;
    m_undoStack->push(new GroupRenameCommand(m_document, groupId, oldName, newName));
}

void MainWindow::resetGroupIdCounter()
{
    m_nextGroupId = 1;
    for (auto *item : m_canvasScene->items()) {
        auto *s = dynamic_cast<ShapeBase*>(item);
        if (s && s->groupId() >= m_nextGroupId)
            m_nextGroupId = s->groupId() + 1;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) event->accept();
    else event->ignore();
}
