#ifndef FILTERS_H
#define FILTERS_H

#include <QImage>

/**
 * @namespace Filters
 * @brief Contains various image processing filters, including functional
 * adjustments and convolution-based effects.
 */
namespace Filters {

/**
 * @brief applyConvolution3x3
 * Applies a 3×3 convolution on the input image using the provided kernel,
 * divisor, and offset.
 *
 * @param image     The input image (converted to RGB32 internally).
 * @param kernel    An odd-sized integer kernel.
 * @param divisor   The value used to divide the summed pixel contributions.
 * @param offset    A bias added after division (useful for emboss or custom
 * shifts).
 * @param anchorX   X-parameter of the anchor.
 * @param anchorY   Y-parameter of the anchor.
 * @return          A new QImage with the convolution applied.
 */
QImage applyConvolution(const QImage &image,
                        const QVector<QVector<int>> &kernel, int divisor,
                        int offset, int anchorX, int anchorY);

/**
 * @brief Inverts the colors of an image.
 * @param image The input image.
 * @return A new image with inverted colors.
 */
QImage invert(const QImage &image);

/**
 * @brief Adjusts the brightness of an image.
 * @param image The input image.
 * @param delta The brightness adjustment value (-255 to 255).
 * @return A new image with adjusted brightness.
 */
QImage adjustBrightness(const QImage &image, int delta);

/**
 * @brief Adjusts the contrast of an image.
 * @param image The input image.
 * @param factor The contrast adjustment factor (>1 increases contrast, <1
 * decreases contrast).
 * @return A new image with adjusted contrast.
 */
QImage adjustContrast(const QImage &image, double factor);

/**
 * @brief Adjusts the gamma of an image.
 * @param image The input image.
 * @param gammaValue The gamma correction value (>1 brightens, <1 darkens).
 * @return A new image with adjusted gamma.
 */
QImage adjustGamma(const QImage &image, double gammaValue);

/**
 * @brief Applies a simple box blur filter.
 * @param image The input image.
 * @return A new image with a 3x3 blur applied.
 */
QImage blur3x3(const QImage &image);

/**
 * @brief Applies a 3x3 Gaussian blur filter.
 * @param image The input image.
 * @return A new image with a Gaussian blur applied.
 */
QImage gaussianBlur3x3(const QImage &image);

/**
 * @brief Applies a sharpening filter to enhance edges.
 * @param image The input image.
 * @return A new image with sharpening applied.
 */
QImage sharpen3x3(const QImage &image);

/**
 * @brief Applies an edge detection filter.
 * @param image The input image.
 * @return A new image with edge detection applied.
 */
QImage edgeDetect3x3(const QImage &image);

/**
 * @brief Applies an emboss effect to the image.
 * @param image The input image.
 * @return A new image with an embossed effect.
 */
QImage emboss3x3(const QImage &image);

/**
 * @brief Applies a median filter to a grayscale image.
 * @param src The input grayscale image.
 * @param kernelSize The size of the median filter window (must be odd).
 * @return A new QImage with the median filter applied.
 */
QImage applyMedianFilter(const QImage &image, int kernelSize = 3);

/**
 * @brief Applies a erosion filter to a grayscale image.
 * @param src The input grayscale image.
 * @param kernelSize The size of the erosion filter window (must be odd).
 * @return A new QImage with the erosion filter applied.
 */
QImage applyErosionFilter(const QImage &image, int kernelSize = 3);

/**
 * @brief Applies a dilation filter to a grayscale image.
 * @param src The input grayscale image.
 * @param kernelSize The size of the dilation filter window (must be odd).
 * @return A new QImage with the dilation filter applied.
 */
QImage applyDilationFilter(const QImage &image, int kernelSize = 3);

} // namespace Filters

#endif // FILTERS_H
