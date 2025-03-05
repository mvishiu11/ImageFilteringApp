#include "ConvolutionEditorWidget.h"
#include <QTableWidget>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QToolButton>
#include <QMenu>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QColor>
#include <QDebug>

ConvolutionEditorWidget::ConvolutionEditorWidget(QWidget *parent)
    : QDockWidget(parent)
{
    // Set title and disable floatable.
    setWindowTitle(tr("Convolution Editor"));
    setFeatures(features() & ~QDockWidget::DockWidgetFloatable);

    QWidget *dockContent = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(dockContent);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    // --- Kernel Size Selection ---
    QHBoxLayout *sizeLayout = new QHBoxLayout;
    QLabel *labelRows = new QLabel(tr("Rows:"), dockContent);
    spinRows = new QSpinBox(dockContent);
    spinRows->setRange(1, 9);
    spinRows->setSingleStep(2);  // Only odd numbers.
    spinRows->setValue(3);
    QLabel *labelCols = new QLabel(tr("Cols:"), dockContent);
    spinCols = new QSpinBox(dockContent);
    spinCols->setRange(1, 9);
    spinCols->setSingleStep(2);
    spinCols->setValue(3);
    sizeLayout->addWidget(labelRows);
    sizeLayout->addWidget(spinRows);
    sizeLayout->addSpacing(10);
    sizeLayout->addWidget(labelCols);
    sizeLayout->addWidget(spinCols);
    mainLayout->addLayout(sizeLayout);

    // --- Anchor Point Selection ---
    QHBoxLayout *anchorLayout = new QHBoxLayout;
    QLabel *labelAnchor = new QLabel(tr("Anchor (X, Y):"), dockContent);
    spinAnchorX = new QSpinBox(dockContent);
    spinAnchorX->setRange(0, spinCols->value() - 1);
    spinAnchorX->setValue(spinCols->value() / 2);
    spinAnchorY = new QSpinBox(dockContent);
    spinAnchorY->setRange(0, spinRows->value() - 1);
    spinAnchorY->setValue(spinRows->value() / 2);
    anchorLayout->addWidget(labelAnchor);
    anchorLayout->addWidget(spinAnchorX);
    anchorLayout->addWidget(spinAnchorY);
    mainLayout->addLayout(anchorLayout);

    // --- Kernel Coefficients Table ---
    tableKernel = new QTableWidget(3, 3, dockContent);
    tableKernel->horizontalHeader()->setVisible(false);
    tableKernel->verticalHeader()->setVisible(false);
    tableKernel->setAlternatingRowColors(true);
    updateKernelTable(3, 3);  // Populate with default values.
    mainLayout->addWidget(tableKernel);
    connect(tableKernel, &QTableWidget::itemChanged,
            this, &ConvolutionEditorWidget::onTableItemChanged);

    // --- Preset Buttons using a QToolButton ---
    btnPresets = new QToolButton(dockContent);
    btnPresets->setText(tr("Presets"));
    btnPresets->setPopupMode(QToolButton::InstantPopup);
    QMenu *presetMenu = new QMenu(btnPresets);
    QAction *actionBlur = presetMenu->addAction(tr("Blur"));
    QAction *actionGaussian = presetMenu->addAction(tr("Gaussian"));
    QAction *actionSharpen = presetMenu->addAction(tr("Sharpen"));
    QAction *actionEdge = presetMenu->addAction(tr("Edge"));
    QAction *actionEmboss = presetMenu->addAction(tr("Emboss"));
    btnPresets->setMenu(presetMenu);
    mainLayout->addWidget(btnPresets);

    // Connect preset actions.
    connect(actionBlur, &QAction::triggered, this, &ConvolutionEditorWidget::onPresetBlurClicked);
    connect(actionGaussian, &QAction::triggered, this, &ConvolutionEditorWidget::onPresetGaussianClicked);
    connect(actionSharpen, &QAction::triggered, this, &ConvolutionEditorWidget::onPresetSharpenClicked);
    connect(actionEdge, &QAction::triggered, this, &ConvolutionEditorWidget::onPresetEdgeClicked);
    connect(actionEmboss, &QAction::triggered, this, &ConvolutionEditorWidget::onPresetEmbossClicked);

    // --- Divisor, Offset, and Auto Divisor ---
    QHBoxLayout *paramLayout = new QHBoxLayout;
    QLabel *labelDivisor = new QLabel(tr("Divisor:"), dockContent);
    lineDivisor = new QLineEdit("1", dockContent);
    lineDivisor->setMaximumWidth(50);
    checkAutoDivisor = new QCheckBox(tr("Auto"), dockContent);
    checkAutoDivisor->setChecked(false);
    QLabel *labelOffset = new QLabel(tr("Offset:"), dockContent);
    lineOffset = new QLineEdit("0", dockContent);
    lineOffset->setMaximumWidth(50);
    paramLayout->addWidget(labelDivisor);
    paramLayout->addWidget(lineDivisor);
    paramLayout->addWidget(checkAutoDivisor);
    paramLayout->addSpacing(10);
    paramLayout->addWidget(labelOffset);
    paramLayout->addWidget(lineOffset);
    mainLayout->addLayout(paramLayout);
    connect(checkAutoDivisor, &QCheckBox::toggled,
            this, &ConvolutionEditorWidget::onAutoDivisorToggled);

    // --- Apply Button ---
    btnApply = new QPushButton(tr("Apply Filter"), dockContent);
    mainLayout->addWidget(btnApply);

    dockContent->setLayout(mainLayout);
    setWidget(dockContent);

    // --- Signal-Slot Connections ---
    connect(spinRows, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ConvolutionEditorWidget::onKernelSizeChanged);
    connect(spinCols, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ConvolutionEditorWidget::onKernelSizeChanged);
    connect(btnApply, &QPushButton::clicked,
            this, &ConvolutionEditorWidget::onApplyButtonClicked);
}

void ConvolutionEditorWidget::updateKernelTable(int rows, int cols)
{
    tableKernel->setRowCount(rows);
    tableKernel->setColumnCount(cols);
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (!tableKernel->item(i, j)) {
                QTableWidgetItem *item = new QTableWidgetItem("0");
                item->setTextAlignment(Qt::AlignCenter);
                tableKernel->setItem(i, j, item);
            }
        }
    }
    spinAnchorX->setRange(0, cols - 1);
    spinAnchorY->setRange(0, rows - 1);
    spinAnchorX->setValue(cols / 2);
    spinAnchorY->setValue(rows / 2);
    tableKernel->resizeColumnsToContents();
    tableKernel->resizeRowsToContents();
}

void ConvolutionEditorWidget::onKernelSizeChanged(int /*value*/)
{
    int rows = spinRows->value();
    int cols = spinCols->value();
    updateKernelTable(rows, cols);
    if (checkAutoDivisor->isChecked()) {
        updateAutoDivisor();
    }
}

void ConvolutionEditorWidget::onApplyButtonClicked()
{
    emit applyConvolutionFilter();
}

QVector<QVector<int>> ConvolutionEditorWidget::getKernel() const
{
    int rows = tableKernel->rowCount();
    int cols = tableKernel->columnCount();
    QVector<QVector<int>> kernel;
    kernel.resize(rows);
    for (int i = 0; i < rows; i++) {
        kernel[i].resize(cols);
        for (int j = 0; j < cols; j++) {
            int value = 0;
            if (tableKernel->item(i, j)) {
                bool ok;
                value = tableKernel->item(i, j)->text().toInt(&ok);
                if (!ok)
                    value = 0;
            }
            kernel[i][j] = value;
        }
    }
    return kernel;
}

int ConvolutionEditorWidget::getDivisor() const
{
    if (checkAutoDivisor->isChecked()) {
        QVector<QVector<int>> kernel = getKernel();
        int sum = 0;
        for (const auto &row : kernel)
            for (int val : row)
                sum += val;
        return (sum != 0) ? sum : 1;
    } else {
        bool ok;
        int div = lineDivisor->text().toInt(&ok);
        return ok ? div : 1;
    }
}

int ConvolutionEditorWidget::getOffset() const
{
    bool ok;
    int off = lineOffset->text().toInt(&ok);
    return ok ? off : 0;
}

QPair<int, int> ConvolutionEditorWidget::getAnchor() const
{
    return qMakePair(spinAnchorX->value(), spinAnchorY->value());
}

void ConvolutionEditorWidget::onTableItemChanged(QTableWidgetItem *item)
{
    bool ok;
    item->text().toInt(&ok);
    if (!ok) {
        item->setBackground(Qt::red);
    } else {
        item->setBackground(Qt::white);
        if (checkAutoDivisor->isChecked()) {
            updateAutoDivisor();
        }
    }
}

void ConvolutionEditorWidget::onAutoDivisorToggled(bool checked)
{
    lineDivisor->setDisabled(checked);
    if (checked) {
        updateAutoDivisor();
    }
}

void ConvolutionEditorWidget::updateAutoDivisor()
{
    QVector<QVector<int>> kernel = getKernel();
    int sum = 0;
    for (const auto &row : kernel)
        for (int val : row)
            sum += val;
    int autoDiv = (sum != 0) ? sum : 1;
    lineDivisor->setText(QString::number(autoDiv));
}

// ---- Preset Buttons Slots ----

void ConvolutionEditorWidget::onPresetBlurClicked()
{
    // Preset: Box Blur (3x3 with all ones)
    spinRows->setValue(3);
    spinCols->setValue(3);
    updateKernelTable(3, 3);
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            tableKernel->item(i, j)->setText("1");
    lineDivisor->setText("9");
    lineOffset->setText("0");
    spinAnchorX->setValue(1);
    spinAnchorY->setValue(1);
    if (checkAutoDivisor->isChecked())
        updateAutoDivisor();
}

void ConvolutionEditorWidget::onPresetGaussianClicked()
{
    // Preset: Gaussian Blur (3x3)
    spinRows->setValue(3);
    spinCols->setValue(3);
    updateKernelTable(3, 3);
    // 1 2 1 / 2 4 2 / 1 2 1, divisor = 16
    tableKernel->item(0, 0)->setText("1");
    tableKernel->item(0, 1)->setText("2");
    tableKernel->item(0, 2)->setText("1");
    tableKernel->item(1, 0)->setText("2");
    tableKernel->item(1, 1)->setText("4");
    tableKernel->item(1, 2)->setText("2");
    tableKernel->item(2, 0)->setText("1");
    tableKernel->item(2, 1)->setText("2");
    tableKernel->item(2, 2)->setText("1");
    lineDivisor->setText("16");
    lineOffset->setText("0");
    spinAnchorX->setValue(1);
    spinAnchorY->setValue(1);
    if (checkAutoDivisor->isChecked())
        updateAutoDivisor();
}

void ConvolutionEditorWidget::onPresetSharpenClicked()
{
    // Preset: Sharpen (3x3)
    spinRows->setValue(3);
    spinCols->setValue(3);
    updateKernelTable(3, 3);
    //  0 -1  0
    // -1  5 -1
    //  0 -1  0
    tableKernel->item(0, 0)->setText("0");
    tableKernel->item(0, 1)->setText("-1");
    tableKernel->item(0, 2)->setText("0");
    tableKernel->item(1, 0)->setText("-1");
    tableKernel->item(1, 1)->setText("5");
    tableKernel->item(1, 2)->setText("-1");
    tableKernel->item(2, 0)->setText("0");
    tableKernel->item(2, 1)->setText("-1");
    tableKernel->item(2, 2)->setText("0");
    lineDivisor->setText("1");
    lineOffset->setText("0");
    spinAnchorX->setValue(1);
    spinAnchorY->setValue(1);
    if (checkAutoDivisor->isChecked())
        updateAutoDivisor();
}

void ConvolutionEditorWidget::onPresetEdgeClicked()
{
    // Preset: Edge Detection (3x3)
    spinRows->setValue(3);
    spinCols->setValue(3);
    updateKernelTable(3, 3);
    //  0  1  0
    //  1 -4  1
    //  0  1  0
    tableKernel->item(0, 0)->setText("0");
    tableKernel->item(0, 1)->setText("1");
    tableKernel->item(0, 2)->setText("0");
    tableKernel->item(1, 0)->setText("1");
    tableKernel->item(1, 1)->setText("-4");
    tableKernel->item(1, 2)->setText("1");
    tableKernel->item(2, 0)->setText("0");
    tableKernel->item(2, 1)->setText("1");
    tableKernel->item(2, 2)->setText("0");
    lineDivisor->setText("1");
    lineOffset->setText("0");
    spinAnchorX->setValue(1);
    spinAnchorY->setValue(1);
    if (checkAutoDivisor->isChecked())
        updateAutoDivisor();
}

void ConvolutionEditorWidget::onPresetEmbossClicked()
{
    // Preset: Emboss (3x3)
    spinRows->setValue(3);
    spinCols->setValue(3);
    updateKernelTable(3, 3);
    // -2 -1  0
    // -1  1  1
    //  0  1  2
    tableKernel->item(0, 0)->setText("-2");
    tableKernel->item(0, 1)->setText("-1");
    tableKernel->item(0, 2)->setText("0");
    tableKernel->item(1, 0)->setText("-1");
    tableKernel->item(1, 1)->setText("1");
    tableKernel->item(1, 2)->setText("1");
    tableKernel->item(2, 0)->setText("0");
    tableKernel->item(2, 1)->setText("1");
    tableKernel->item(2, 2)->setText("2");
    lineDivisor->setText("1");
    lineOffset->setText("128");
    spinAnchorX->setValue(1);
    spinAnchorY->setValue(1);
    if (checkAutoDivisor->isChecked())
        updateAutoDivisor();
}
