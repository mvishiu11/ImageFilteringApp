#ifndef DITHERINGANDQUANTIZATION_H
#define DITHERINGANDQUANTIZATION_H

#include <QImage>
#include <QVector>

namespace DitheringAndQuantization {
/**
 * @brief Applies an Ordered Dithering algorithm to a color image.
 *
 * This function applies ordered dithering to each color channel independently.
 * A threshold (Bayer) matrix of the specified size is used to decide whether to
 * round a channel value up or down based on its fractional quantization error.
 *
 * @param image The input color QImage.
 * @param thresholdMapSize The size of the threshold matrix (allowed values: 2,
 * 3, 4, or 6).
 * @param levelsPerChannel The number of quantization levels per channel.
 * @return A new QImage with the ordered dithering applied.
 */
QImage applyOrderedDithering(const QImage &image, int thresholdMapSize,
                             int levelsPerChannel);

/**
 * @brief Applies the Popularity Color Quantization algorithm to a color image.
 *
 * This algorithm selects the most frequent colors in the image (up to the
 * specified number) and then maps each pixel to the nearest color in the
 * resulting palette (using Euclidean distance in RGB space).
 *
 * @param image The input color QImage.
 * @param numColors The number of colors to reduce the image to.
 * @return A new QImage with the popularity quantization applied.
 */
QImage applyPopularityQuantization(const QImage &image, int numColors);

} // namespace DitheringAndQuantization

#endif // DITHERINGANDQUANTIZATION_H
