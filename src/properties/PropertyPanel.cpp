#include "PropertyPanel.h"
#include "ui_PropertyPanel.h"
#include "canvas/CanvasScene.h"
#include "shapes/ShapeBase.h"
#include "commands/StyleCommand.h"
#include "commands/TransformCommand.h"
#include "commands/ResizeCommand.h"
#include <QColorDialog>
#include <QGraphicsItem>

PropertyPanel::PropertyPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PropertyPanel)
{
    ui->setupUi(this);

    // 填充色按钮
    connect(ui->fillColorBtn, &QPushButton::clicked,
            this, &PropertyPanel::onFillColorClicked);

    // 描边色按钮
    connect(ui->strokeColorBtn, &QPushButton::clicked,
            this, &PropertyPanel::onStrokeColorClicked);

    // 线宽
    connect(ui->strokeWidthSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PropertyPanel::onStrokeWidthChanged);

    // 位置与大小
    connect(ui->xSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onPositionChanged);
    connect(ui->ySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onPositionChanged);
    connect(ui->wSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onSizeChanged);
    connect(ui->hSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PropertyPanel::onSizeChanged);

    clearSelection();
}

PropertyPanel::~PropertyPanel()
{
    delete ui;
}

void PropertyPanel::setScene(CanvasScene *scene)
{
    m_scene = scene;
}

void PropertyPanel::clearSelection()
{
    m_updating = true;
    ui->fillColorBtn->setStyleSheet("background-color: #ffffff;");
    ui->strokeColorBtn->setStyleSheet("background-color: #000000;");
    ui->strokeWidthSpin->setValue(2);
    ui->xSpin->setValue(0);
    ui->ySpin->setValue(0);
    ui->wSpin->setValue(0);
    ui->hSpin->setValue(0);
    ui->fillGroup->setEnabled(false);
    ui->strokeGroup->setEnabled(false);
    ui->transformGroup->setEnabled(false);
    m_updating = false;
}

void PropertyPanel::onSelectionChanged()
{
    if (!m_scene) return;

    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) {
        clearSelection();
        return;
    }

    ShapeBase *shape = dynamic_cast<ShapeBase*>(selected.first());
    if (shape) {
        updateFromShape(shape);
        ui->fillGroup->setEnabled(true);
        ui->strokeGroup->setEnabled(true);
        ui->transformGroup->setEnabled(true);
    }
}

void PropertyPanel::updateFromShape(ShapeBase *shape)
{
    m_updating = true;

    m_currentStyle = shape->shapeStyle();

    // 填充色
    ui->fillColorBtn->setStyleSheet(
        QString("background-color: %1;")
            .arg(m_currentStyle.fillColor.name()));

    // 描边色
    ui->strokeColorBtn->setStyleSheet(
        QString("background-color: %1;")
            .arg(m_currentStyle.strokeColor.name()));

    // 线宽
    ui->strokeWidthSpin->setValue(static_cast<int>(m_currentStyle.strokeWidth));

    // 位置
    ui->xSpin->setValue(shape->pos().x());
    ui->ySpin->setValue(shape->pos().y());

    // 大小 - 从 boundingRect 获取
    QRectF br = shape->boundingRect();
    ui->wSpin->setValue(br.width());
    ui->hSpin->setValue(br.height());

    m_updating = false;
}

void PropertyPanel::applyStyleToSelection()
{
    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    m_scene->beginUndoMacro(tr("更改样式"));
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (!shape) continue;
        ShapeStyle oldStyle = shape->shapeStyle();
        if (oldStyle == m_currentStyle) continue;
        m_scene->pushUndoCommand(new StyleCommand(shape, oldStyle, m_currentStyle));
    }
    m_scene->endUndoMacro();
    m_scene->setModified(true);
}

void PropertyPanel::applyPositionToSelection()
{
    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    QPointF newPos(ui->xSpin->value(), ui->ySpin->value());
    m_scene->beginUndoMacro(tr("移动图形"));
    for (auto *item : selected) {
        if (item->pos() == newPos) continue;
        m_scene->pushUndoCommand(new TransformCommand(
            item, item->pos(), item->transform(), newPos, item->transform()));
    }
    m_scene->endUndoMacro();
    m_scene->setModified(true);
}

void PropertyPanel::applySizeToSelection()
{
    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.isEmpty()) return;

    QSizeF newSize(ui->wSpin->value(), ui->hSpin->value());
    m_scene->beginUndoMacro(tr("缩放图形"));
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (!shape || shape->size().isEmpty()) continue;
        if (shape->size() == newSize) continue;
        m_scene->pushUndoCommand(new ResizeCommand(
            shape, shape->pos(), shape->size(), shape->pos(), newSize));
    }
    m_scene->endUndoMacro();
    m_scene->setModified(true);
}

// ===== 颜色 =====

void PropertyPanel::onFillColorClicked()
{
    QColor color = QColorDialog::getColor(m_currentStyle.fillColor, this, tr("选择填充色"));
    if (!color.isValid()) return;

    m_currentStyle.fillColor = color;
    ui->fillColorBtn->setStyleSheet(
        QString("background-color: %1;").arg(color.name()));
    applyStyleToSelection();
}

void PropertyPanel::onStrokeColorClicked()
{
    QColor color = QColorDialog::getColor(m_currentStyle.strokeColor, this, tr("选择描边色"));
    if (!color.isValid()) return;

    m_currentStyle.strokeColor = color;
    ui->strokeColorBtn->setStyleSheet(
        QString("background-color: %1;").arg(color.name()));
    applyStyleToSelection();
}

void PropertyPanel::onStrokeWidthChanged(int width)
{
    if (m_updating) return;
    m_currentStyle.strokeWidth = width;
    applyStyleToSelection();
}

void PropertyPanel::onPositionChanged()
{
    if (m_updating) return;
    applyPositionToSelection();
}

void PropertyPanel::onSizeChanged()
{
    if (m_updating) return;
    applySizeToSelection();
}

void PropertyPanel::blockSignals(bool block)
{
    ui->xSpin->blockSignals(block);
    ui->ySpin->blockSignals(block);
    ui->wSpin->blockSignals(block);
    ui->hSpin->blockSignals(block);
    ui->strokeWidthSpin->blockSignals(block);
}
