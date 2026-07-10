#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CanvasView;
class CanvasScene;
class ToolManager;
class LayerPanel;
class PropertyPanel;
class Document;
class QUndoStack;
class QLabel;
class FileManager;

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

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBars();
    void setupDockWidgets();
    void setupStatusBar();
    void setupConnections();
    void updateStatusBar();
    void updateWindowTitle();
    bool maybeSave();

    Ui::MainWindow *ui = nullptr;

    // 核心组件
    CanvasView    *m_canvasView   = nullptr;
    CanvasScene   *m_canvasScene  = nullptr;
    ToolManager   *m_toolManager  = nullptr;
    LayerPanel    *m_layerPanel   = nullptr;
    PropertyPanel *m_propertyPanel = nullptr;
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

    // 停靠窗口
    QDockWidget *m_layerDock    = nullptr;
    QDockWidget *m_propertyDock = nullptr;

    // 文字工具栏
    QToolBar *m_textToolBar = nullptr;
};

#endif // MAINWINDOW_H
