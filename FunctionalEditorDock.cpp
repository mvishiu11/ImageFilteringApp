#include "FunctionalEditorDock.h"
#include "FunctionEditorCanvas.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

FunctionalEditorDock::FunctionalEditorDock(QWidget *parent)
    : QDockWidget(parent),
    m_canvas(new FunctionEditorCanvas(this))
{
    setWindowTitle(tr("Functional Editor"));

    QWidget *dockContent = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(dockContent);

    layout->addWidget(m_canvas);

    QPushButton *btnApply = new QPushButton(tr("Apply"), dockContent);
    connect(btnApply, &QPushButton::clicked, this, &FunctionalEditorDock::onApplyClicked);
    layout->addWidget(btnApply);

    dockContent->setLayout(layout);
    setWidget(dockContent);
}

void FunctionalEditorDock::onApplyClicked()
{
    QVector<int> lut = generateLUT();
    emit functionApplied(lut);
}

QVector<int> FunctionalEditorDock::generateLUT() const
{
    return m_canvas->buildLookupTable();
}
