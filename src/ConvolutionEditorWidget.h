#ifndef CONVOLUTIONEDITORWIDGET_H
#define CONVOLUTIONEDITORWIDGET_H

#include <QDockWidget>
#include <QPair>
#include <QVector>

/**
 * @brief The ConvolutionEditorWidget class
 *
 * This dockable widget provides a user interface for editing convolution filter
 * parameters. It allows the user to:
 * - Select the kernel size (rows and columns; only odd numbers are allowed).
 * - Edit kernel coefficients in a table with error checking and cell
 * highlighting.
 * - Specify the divisor and offset (or auto-calculate the divisor via a
 * checkbox).
 * - Choose the anchor point (which element of the kernel overlays the processed
 * pixel), with the anchor cell highlighted.
 * - Quickly preset common convolution filters via preset buttons.
 *
 * When the user clicks "Apply Filter," the widget emits the
 * applyConvolutionFilter signal.
 */
class ConvolutionEditorWidget : public QDockWidget {
  Q_OBJECT
public:
  /**
   * @brief Constructs a ConvolutionEditorWidget.
   * @param parent The parent widget (default is nullptr).
   */
  explicit ConvolutionEditorWidget(QWidget *parent = nullptr);

  /**
   * @brief Retrieves the convolution kernel entered by the user.
   * @return A 2D QVector representing the kernel coefficients.
   */
  QVector<QVector<int>> getKernel() const;

  /**
   * @brief Retrieves the divisor value.
   * @return The divisor as an integer.
   */
  int getDivisor() const;

  /**
   * @brief Retrieves the offset value.
   * @return The offset as an integer.
   */
  int getOffset() const;

  /**
   * @brief Retrieves the anchor point for the kernel.
   * @return A QPair where the first element is the X coordinate and the second
   * is the Y coordinate.
   */
  QPair<int, int> getAnchor() const;

signals:
  /**
   * @brief Emitted when the user clicks the "Apply Filter" button.
   */
  void applyConvolutionFilter();

private slots:
  /**
   * @brief Slot called when the kernel size (rows or columns) is changed.
   * @param size The new size (unused parameter from QSpinBox signal).
   */
  void onKernelSizeChanged(int size);

  /**
   * @brief Slot called when the "Apply Filter" button is clicked.
   */
  void onApplyButtonClicked();

  /**
   * @brief Slot called when a cell in the kernel table is changed.
   * @param item Pointer to the changed table widget item.
   */
  void onTableItemChanged(class QTableWidgetItem *item);

  /**
   * @brief Slot called when the auto divisor checkbox is toggled.
   * @param checked True if auto divisor is enabled.
   */
  void onAutoDivisorToggled(bool checked);

  // Preset button slots:
  void onPresetBlurClicked();
  void onPresetGaussianClicked();
  void onPresetSharpenClicked();
  void onPresetEdgeClicked();
  void onPresetEmbossClicked();

private:
  // UI Elements:
  class QTableWidget *tableKernel; ///< Table for entering kernel coefficients.
  class QSpinBox *spinRows;        ///< SpinBox for kernel row count.
  class QSpinBox *spinCols;        ///< SpinBox for kernel column count.
  class QSpinBox *spinAnchorX;     ///< SpinBox for anchor X coordinate.
  class QSpinBox *spinAnchorY;     ///< SpinBox for anchor Y coordinate.
  class QLineEdit *lineDivisor;    ///< LineEdit for entering the divisor.
  class QLineEdit *lineOffset;     ///< LineEdit for entering the offset.
  class QPushButton *btnApply;     ///< Button to apply the convolution filter.
  class QCheckBox
      *checkAutoDivisor; ///< Checkbox to auto-calculate the divisor.

  // Preset tool button
  class QToolButton *btnPresets;

  /**
   * @brief Updates the kernel table to reflect the specified dimensions.
   * @param rows Number of rows.
   * @param cols Number of columns.
   */
  void updateKernelTable(int rows, int cols);

  /**
   * @brief Updates the divisor field automatically if auto divisor is enabled.
   */
  void updateAutoDivisor();
};

#endif // CONVOLUTIONEDITORWIDGET_H
