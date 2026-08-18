#ifndef PROPERTYPANEL_H
#define PROPERTYPANEL_H

#include <QWidget>
#include "style/ShapeStyle.h"

class ShapeBase;
class CanvasScene;

namespace Ui {
class PropertyPanel;
}

class PropertyPanel : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyPanel(QWidget *parent = nullptr);
    ~PropertyPanel();

    void setScene(CanvasScene *scene);

public slots:
    void onSelectionChanged();
    void clearSelection();

private slots:
    void onFillColorClicked();
    void onStrokeColorClicked();
    void onStrokeWidthChanged(int width);
    void onPositionChanged();
    void onSizeChanged();

private:
    void updateFromShape(ShapeBase *shape);
    void applyStyleToSelection();
    void applyPositionToSelection();
    void applySizeToSelection();
    void blockSignals(bool block);

    Ui::PropertyPanel *ui;
    CanvasScene *m_scene = nullptr;
    ShapeStyle m_currentStyle;
    bool m_updating = false;
};

#endif // PROPERTYPANEL_H
