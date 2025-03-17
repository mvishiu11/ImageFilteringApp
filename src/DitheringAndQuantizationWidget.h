#ifndef DITHERINGANDQUANTIZATIONWIDGET_H
#define DITHERINGANDQUANTIZATIONWIDGET_H

#include <QDockWidget>
#include <QImage>
#include <QVector>

/**
 * @brief The DitheringQuantizationWidget class
 *
 * This dockable widget provides a graphical user interface for applying
 * ordered dithering and popularity color quantization algorithms to an image.
 *
 * For Ordered Dithering, the user can select:
 * - The size of the threshold map (2, 3, 4, or 6).
 * - The number of quantization levels per color channel.
 *
 * For Popularity Quantization, the user can select:
 * - The number of colors in the resulting image.
 *
 * The widget emits signals when the user clicks the corresponding apply
 * buttons.
 */
class DitheringQuantizationWidget : public QDockWidget {
  Q_OBJECT
public:
  /**
   * @brief Constructs the DitheringQuantizationWidget.
   * @param parent Parent widget (default is nullptr).
   */
  explicit DitheringQuantizationWidget(QWidget *parent = nullptr);

signals:
  /**
   * @brief Emitted when the user requests to apply ordered dithering.
   * @param thresholdMapSize The selected threshold map size.
   * @param levelsPerChannel The number of quantization levels per channel.
   */
  void applyOrderedDitheringRequested(int thresholdMapSize,
                                      int levelsPerChannel);

  /**
   * @brief Emitted when the user requests to apply popularity quantization.
   * @param numColors The desired number of colors.
   */
  void applyPopularityQuantizationRequested(int numColors);

private slots:
  void onApplyOrderedDitheringClicked();
  void onApplyPopularityQuantizationClicked();

private:
  // Controls for Ordered Dithering.
  class QComboBox
      *comboThresholdSize;    ///< Dropdown to select threshold map size.
  class QSpinBox *spinLevels; ///< SpinBox for quantization levels per channel.
  class QPushButton *btnApplyDithering; ///< Button to apply ordered dithering.

  // Controls for Popularity Quantization.
  class QSpinBox *spinNumColors; ///< SpinBox for number of colors.
  class QPushButton
      *btnApplyQuantization; ///< Button to apply popularity quantization.
};

#endif // DITHERINGANDQUANTIZATIONWIDGET_H
