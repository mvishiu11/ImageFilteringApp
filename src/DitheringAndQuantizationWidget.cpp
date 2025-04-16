#include "DitheringAndQuantizationWidget.h"
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

DitheringQuantizationWidget::DitheringQuantizationWidget(QWidget *parent)
    : QDockWidget(parent) {
  setWindowTitle(tr("Dithering and Quantization"));

  QWidget *dockContent = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(dockContent);
  mainLayout->setContentsMargins(10, 10, 10, 10);
  mainLayout->setSpacing(15);

  // --- Ordered Dithering Section ---
  QLabel *labelDithering = new QLabel(tr("Ordered Dithering"), dockContent);
  labelDithering->setStyleSheet("font-weight: bold; font-size: 11pt;");
  mainLayout->addWidget(labelDithering);

  QHBoxLayout *ditheringLayout = new QHBoxLayout;
  QLabel *labelThreshold = new QLabel(tr("Threshold Map Size:"), dockContent);
  comboThresholdSize = new QComboBox(dockContent);
  comboThresholdSize->addItem("2", 2);
  comboThresholdSize->addItem("3", 3);
  comboThresholdSize->addItem("4", 4);
  comboThresholdSize->addItem("6", 6);
  QLabel *labelLevels = new QLabel(tr("Levels/Channel:"), dockContent);
  spinLevels = new QSpinBox(dockContent);
  spinLevels->setRange(2, 256);
  spinLevels->setValue(8);
  spinLevels->setSuffix(" lvl");

  ditheringLayout->addWidget(labelThreshold);
  ditheringLayout->addWidget(comboThresholdSize);
  ditheringLayout->addSpacing(10);
  ditheringLayout->addWidget(labelLevels);
  ditheringLayout->addWidget(spinLevels);
  mainLayout->addLayout(ditheringLayout);

  btnApplyDithering =
      new QPushButton(tr("Apply Ordered Dithering"), dockContent);
  mainLayout->addWidget(btnApplyDithering);
  connect(btnApplyDithering, &QPushButton::clicked, this,
          &DitheringQuantizationWidget::onApplyOrderedDitheringClicked);

  btnApplyDitheringYCbCr =
      new QPushButton(tr("Apply Ordered Dithering in YCbCr"), dockContent);
  mainLayout->addWidget(btnApplyDitheringYCbCr);
  connect(btnApplyDitheringYCbCr, &QPushButton::clicked, this,
          &DitheringQuantizationWidget::onApplyOrderedDitheringYCbCrClicked);

  // --- Separator ---
  QFrame *separator = new QFrame(dockContent);
  separator->setFrameShape(QFrame::HLine);
  separator->setFrameShadow(QFrame::Sunken);
  mainLayout->addWidget(separator);

  // --- Popularity Quantization Section ---
  QLabel *labelQuantization =
      new QLabel(tr("Popularity Quantization"), dockContent);
  labelQuantization->setStyleSheet("font-weight: bold; font-size: 11pt;");
  mainLayout->addWidget(labelQuantization);

  QHBoxLayout *quantLayout = new QHBoxLayout;
  QLabel *labelNumColors = new QLabel(tr("Number of Colors:"), dockContent);
  spinNumColors = new QSpinBox(dockContent);
  spinNumColors->setRange(2, 256);
  spinNumColors->setValue(16);
  spinNumColors->setSuffix(" colors");
  quantLayout->addWidget(labelNumColors);
  quantLayout->addWidget(spinNumColors);
  mainLayout->addLayout(quantLayout);

  btnApplyQuantization =
      new QPushButton(tr("Apply Popularity Quantization"), dockContent);
  mainLayout->addWidget(btnApplyQuantization);
  connect(btnApplyQuantization, &QPushButton::clicked, this,
          &DitheringQuantizationWidget::onApplyPopularityQuantizationClicked);

  dockContent->setLayout(mainLayout);
  setWidget(dockContent);
}

void DitheringQuantizationWidget::onApplyOrderedDitheringClicked() {
  int thresholdMapSize = comboThresholdSize->currentData().toInt();
  int levelsPerChannel = spinLevels->value();
  emit applyOrderedDitheringRequested(thresholdMapSize, levelsPerChannel);
}

void DitheringQuantizationWidget::onApplyOrderedDitheringYCbCrClicked() {
  int thresholdMapSize = comboThresholdSize->currentData().toInt();
  int levelsPerChannel = spinLevels->value();
  emit applyOrderedDitheringYCbCrRequested(thresholdMapSize, levelsPerChannel);
}

void DitheringQuantizationWidget::onApplyPopularityQuantizationClicked() {
  int numColors = spinNumColors->value();
  emit applyPopularityQuantizationRequested(numColors);
}
