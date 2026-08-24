#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonArray>

class CanvasView;
class CanvasScene;
class ToolManager;
class LayerPanel;
class PropertyPanel;
class Document;
class QUndoStack;
class QLabel;
class FileManager;
class QPushButton;
class QComboBox;
class QDoubleSpinBox;
class QToolButton;
class HistoryPanel;
class ShapeBase;
class Layer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件菜单
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onExportSvg();
    void onExportPng();

    // 编辑菜单
    void onDeleteSelected();
    void onSelectAll();
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void duplicateSelection();
    void nudgeSelected(qreal dx, qreal dy);

    // 图层操作
    void onAddLayerRequested();
    void onRemoveLayerRequested(int index);
    void onMoveLayerRequested(int from, int to);
    void onRenameLayerRequested(int index, const QString &newName);
    void onRenameGroupRequested(qint64 groupId, const QString &newName);

private:
    void setupMenuBar();
    void setupToolBars();
    void setupToolBarEdit();
    void setupToolBarDraw();
    void setupToolBarShape();
    void setupToolBarText();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();
    void updateStatusBar();
    void updateWindowTitle();
    bool maybeSave();
    QJsonArray serializeSelection() const;
    void insertShapes(const QJsonArray &arr, const QPointF &offset = QPointF(20, 20));
    void pasteInPlace();
    void pasteAt(const QPointF &pos);
    void clearCanvas();

    // 层级与对齐
    void bringToFront();
    void sendToBack();
    void bringForward();
    void sendBackward();
    enum AlignMode { AlignLeft, AlignHCenter, AlignRight, AlignTop, AlignVCenter, AlignBottom };
    void alignSelected(AlignMode mode);
    QList<ShapeBase*> selectedShapes() const;
    void restackByLayers();
    void pushLayerReorder(const QList<Layer*> &layers, const QList<QList<ShapeBase*>> &newOrders, const QString &text);
    void showContextMenu(const QPointF &scenePos, const QPoint &globalPos);
    bool canPaste() const;

    // 编组
    void groupSelected();
    void ungroupSelected();
    void resetGroupIdCounter();

    Ui::MainWindow *ui = nullptr;

    // 核心组件
    CanvasView    *m_canvasView   = nullptr;
    CanvasScene   *m_canvasScene  = nullptr;
    ToolManager   *m_toolManager  = nullptr;
    LayerPanel    *m_layerPanel   = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
    HistoryPanel  *m_historyPanel  = nullptr;
    Document      *m_document     = nullptr;
    FileManager   *m_fileManager  = nullptr;
    QUndoStack    *m_undoStack    = nullptr;

    // 状态栏标签
    QLabel *m_statusLabel  = nullptr;
    QLabel *m_zoomLabel    = nullptr;
    QLabel *m_layerLabel   = nullptr;

    // 当前文件路径
    QString m_currentFilePath;
    bool    m_modified = false;
    qint64  m_nextGroupId = 1;   // 编组 ID 计数器

    // 停靠窗口
    QDockWidget *m_layerDock    = nullptr;
    QDockWidget *m_propertyDock = nullptr;
    QDockWidget *m_historyDock  = nullptr;

    // 文字工具栏
    QToolBar *m_textToolBar = nullptr;
    // 图形子工具栏
    QToolBar *m_shapeToolBar = nullptr;

    // 通用描边控件（主工具栏）
    QPushButton *m_strokeColorBtn = nullptr;
    QComboBox *m_strokeStyleCombo = nullptr;
    QDoubleSpinBox *m_strokeWidthSpin = nullptr;

    // 尺子方向切换按钮
    QToolButton *m_rulerOrientationBtn = nullptr;
};

#endif // MAINWINDOW_H
