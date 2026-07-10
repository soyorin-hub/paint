#include "PropertyPanel.h"
#include "ui_PropertyPanel.h"
#include "canvas/CanvasScene.h"
#include "shapes/ShapeBase.h"
#include "shapes/RectShape.h"
#include "shapes/EllipseShape.h"
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
    ui->fillColorBtn->setStyleSheet(
        "background-color: #ffffff; border: 1px solid #888; border-radius: 2px;");
    ui->strokeColorBtn->setStyleSheet(
        "background-color: #000000; border: 1px solid #888; border-radius: 2px;");
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
        QString("background-color: %1; border: 1px solid #888; border-radius: 2px;")
            .arg(m_currentStyle.fillColor.name()));

    // 描边色
    ui->strokeColorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid #888; border-radius: 2px;")
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

void PropertyPanel::applyToShape(ShapeBase *shape)
{
    if (!shape) return;
    shape->setShapeStyle(m_currentStyle);
    m_scene->setModified(true);
}

// ===== 颜色 =====

void PropertyPanel::onFillColorClicked()
{
    QColor color = QColorDialog::getColor(m_currentStyle.fillColor, this, tr("选择填充色"));
    if (!color.isValid()) return;

    m_currentStyle.fillColor = color;
    ui->fillColorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid #888; border-radius: 2px;")
            .arg(color.name()));

    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (shape) applyToShape(shape);
    }
}

void PropertyPanel::onStrokeColorClicked()
{
    QColor color = QColorDialog::getColor(m_currentStyle.strokeColor, this, tr("选择描边色"));
    if (!color.isValid()) return;

    m_currentStyle.strokeColor = color;
    ui->strokeColorBtn->setStyleSheet(
        QString("background-color: %1; border: 1px solid #888; border-radius: 2px;")
            .arg(color.name()));

    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (shape) applyToShape(shape);
    }
}

void PropertyPanel::onStrokeWidthChanged(int width)
{
    if (m_updating) return;
    m_currentStyle.strokeWidth = width;

    if (!m_scene) return;
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    for (auto *item : selected) {
        ShapeBase *shape = dynamic_cast<ShapeBase*>(item);
        if (shape) applyToShape(shape);
    }
}

void PropertyPanel::onPositionChanged()
{
    if (m_updating || !m_scene) return;

    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    for (auto *item : selected) {
        item->setPos(ui->xSpin->value(), ui->ySpin->value());
        m_scene->setModified(true);
    }
}

void PropertyPanel::onSizeChanged()
{
    if (m_updating || !m_scene) return;

    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    for (auto *item : selected) {
        // 尝试设置矩形大小（对矩形和椭圆有效）
        auto *rectShape = dynamic_cast<RectShape*>(item);
        auto *ellipseShape = dynamic_cast<EllipseShape*>(item);
        if (rectShape) {
            rectShape->setRect(QRectF(0, 0, ui->wSpin->value(), ui->hSpin->value()));
        } else if (ellipseShape) {
            ellipseShape->setRect(QRectF(0, 0, ui->wSpin->value(), ui->hSpin->value()));
        }
        m_scene->setModified(true);
    }
}

void PropertyPanel::blockSignals(bool block)
{
    ui->xSpin->blockSignals(block);
    ui->ySpin->blockSignals(block);
    ui->wSpin->blockSignals(block);
    ui->hSpin->blockSignals(block);
    ui->strokeWidthSpin->blockSignals(block);
}
