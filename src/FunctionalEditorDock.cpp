#include "FunctionalEditorDock.h"
#include "FunctionEditorCanvas.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAbstractSpinBox>
#include <QLabel>

FunctionalEditorDock::FunctionalEditorDock(QWidget *parent)
    : QDockWidget(parent),
    m_canvas(new FunctionEditorCanvas(this))
{
    setWindowTitle(tr("Functional Editor"));

    QWidget *dockContent = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(dockContent);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(10);

    // --- The 256x256 canvas ---
    QHBoxLayout *canvasLayout = new QHBoxLayout;
    canvasLayout->addStretch();
    canvasLayout->addWidget(m_canvas);
    canvasLayout->addStretch();
    layout->addLayout(canvasLayout);

    // --- Apply and Reset Buttons ---
    QPushButton *btnApply = new QPushButton(tr("Apply"), dockContent);
    btnApply->setToolTip(tr("Apply the current function to the image"));
    connect(btnApply, &QPushButton::clicked, this, &FunctionalEditorDock::onApplyClicked);
    layout->addWidget(btnApply);

    QPushButton *btnReset = new QPushButton(tr("Reset"), dockContent);
    btnReset->setToolTip(tr("Reset the function to the default identity"));
    connect(btnReset, &QPushButton::clicked, this, &FunctionalEditorDock::onResetClicked);
    layout->addWidget(btnReset);

    dockContent->setLayout(layout);
    setWidget(dockContent);
}

void FunctionalEditorDock::setInitialBrightnessCurve(int delta, int samplePoints)
{
    m_canvas->setCurveForBrightness(delta, samplePoints);
}

void FunctionalEditorDock::setInitialContrastCurve(double factor, int samplePoints)
{
    m_canvas->setCurveForContrast(factor, samplePoints);
}

void FunctionalEditorDock::setInitialInvertCurve()
{
    m_canvas->setCurveForInvert();
}

void FunctionalEditorDock::onApplyClicked()
{
    QVector<int> lut = generateLUT();
    emit functionApplied(lut);
}

void FunctionalEditorDock::onResetClicked()
{
    m_canvas->resetPoints();
}

QVector<int> FunctionalEditorDock::generateLUT() const
{
    return m_canvas->buildLookupTable();
}
