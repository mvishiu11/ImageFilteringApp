#include "filters.h"
#include <QtMath>
#include <algorithm>

namespace { // An anonymous namespace to group the convolution helpers

    /**
     * @brief applyConvolution3x3
     * Applies a 3×3 convolution on the input image using the provided kernel, divisor, and offset.
     *
     * @param image     The input image (converted to RGB32 internally).
     * @param kernel    An odd-sized integer kernel.
     * @param divisor   The value used to divide the summed pixel contributions.
     * @param offset    A bias added after division (useful for emboss or custom shifts).
     * @param anchorX   X-parameter of the anchor.
     * @param anchorY   Y-parameter of the anchor.
     * @return          A new QImage with the convolution applied.
     */
    QImage applyConvolution(const QImage &image,
                            const QVector<QVector<int>> &kernel,
                            int divisor, int offset,
                            int anchorX, int anchorY)
    {
        QImage src = image.convertToFormat(QImage::Format_RGB32);
        QImage dst(src.size(), QImage::Format_RGB32);

        // Safety: avoid division by 0.
        if (divisor == 0)
            divisor = 1;

        int kRows = kernel.size();
        if (kRows == 0)
            return src;
        int kCols = kernel[0].size();

        // Loop over every pixel.
        for (int y = 0; y < src.height(); y++) {
            for (int x = 0; x < src.width(); x++) {
                int sumR = 0, sumG = 0, sumB = 0;
                // Loop over kernel rows and columns.
                for (int ky = 0; ky < kRows; ky++) {
                    for (int kx = 0; kx < kCols; kx++) {
                        int pixelX = x + kx - anchorX;
                        int pixelY = y + ky - anchorY;
                        int factor = kernel[ky][kx];

                        // Use zero if pixel is out-of-bound.
                        if (pixelX >= 0 && pixelX < src.width() &&
                            pixelY >= 0 && pixelY < src.height()) {
                            QRgb pixel = src.pixel(pixelX, pixelY);
                            sumR += qRed(pixel) * factor;
                            sumG += qGreen(pixel) * factor;
                            sumB += qBlue(pixel) * factor;
                        }
                        // Else: out-of-bound contributes 0.
                    }
                }
                int outR = std::clamp((sumR / divisor) + offset, 0, 255);
                int outG = std::clamp((sumG / divisor) + offset, 0, 255);
                int outB = std::clamp((sumB / divisor) + offset, 0, 255);
                dst.setPixel(x, y, qRgb(outR, outG, outB));
            }
        }

        return dst;
    }

}

namespace Filters
{

//--------------------//
// Functional Filters //
//--------------------//
QImage invert(const QImage &image) {
    QImage result = image.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = result.pixel(x, y);
            int red   = 255 - qRed(pixel);
            int green = 255 - qGreen(pixel);
            int blue  = 255 - qBlue(pixel);
            result.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return result;
}

QImage adjustBrightness(const QImage &image, int delta) {
    QImage result = image.convertToFormat(QImage::Format_RGB32);
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = result.pixel(x, y);
            int red   = qBound(0, qRed(pixel) + delta, 255);
            int green = qBound(0, qGreen(pixel) + delta, 255);
            int blue  = qBound(0, qBlue(pixel) + delta, 255);
            result.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return result;
}

QImage adjustContrast(const QImage &image, double factor) {
    // factor > 1 -> higher contrast, factor < 1 -> lower contrast
    QImage result = image.convertToFormat(QImage::Format_RGB32);
    double midpoint = 128.0;
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = result.pixel(x, y);
            int red   = qBound(0, static_cast<int>((qRed(pixel)   - midpoint) * factor + midpoint),   255);
            int green = qBound(0, static_cast<int>((qGreen(pixel) - midpoint) * factor + midpoint), 255);
            int blue  = qBound(0, static_cast<int>((qBlue(pixel)  - midpoint) * factor + midpoint),  255);
            result.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return result;
}

QImage adjustGamma(const QImage &image, double gammaValue) {
    QImage result = image.convertToFormat(QImage::Format_RGB32);
    // Precompute a lookup table
    unsigned char gammaLUT[256];
    for (int i = 0; i < 256; ++i) {
        gammaLUT[i] = qBound(0, static_cast<int>(255.0 * qPow(i / 255.0, 1.0 / gammaValue)), 255);
    }

    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QRgb pixel = result.pixel(x, y);
            int red   = gammaLUT[qRed(pixel)];
            int green = gammaLUT[qGreen(pixel)];
            int blue  = gammaLUT[qBlue(pixel)];
            result.setPixel(x, y, qRgb(red, green, blue));
        }
    }
    return result;
}

//------------------//
// Convolution 3×3  //
//------------------//

QImage blur3x3(const QImage &image) {
    // Simple box blur kernel
    // 1 1 1
    // 1 1 1
    // 1 1 1
    QVector<QVector<int>> kernel = {
        { 1, 1, 1 },
        { 1, 1, 1 },
        { 1, 1, 1 }
    };
    return applyConvolution(image, kernel, /*divisor*/ 9, /*offset*/ 0, 1, 1);
}

QImage gaussianBlur3x3(const QImage &image) {
    // Basic 3x3 Gaussian kernel
    // 1 2 1
    // 2 4 2
    // 1 2 1
    QVector<QVector<int>> kernel = {
        { 1, 2, 1 },
        { 2, 4, 2 },
        { 1, 2, 1 }
    };
    return applyConvolution(image, kernel, /*divisor*/ 16, /*offset*/ 0, 1, 1);
}

QImage sharpen3x3(const QImage &image) {
    // A common sharpen kernel
    //  0 -1  0
    // -1  5 -1
    //  0 -1  0
    QVector<QVector<int>> kernel = {
        {  0, -1,  0 },
        { -1,  5, -1 },
        {  0, -1,  0 }
    };
    return applyConvolution(image, kernel, /*divisor*/ 1, /*offset*/ 0, 1, 1);
}

QImage edgeDetect3x3(const QImage &image) {
    // A simple edge detection kernel
    //  0  1  0
    //  1 -4  1
    //  0  1  0
    QVector<QVector<int>> kernel = {
        {  0,  1,  0 },
        {  1, -4,  1 },
        {  0,  1,  0 }
    };
    return applyConvolution(image, kernel, /*divisor*/ 1, /*offset*/ 0, 1, 1);
}

QImage emboss3x3(const QImage &image) {
    // A basic emboss kernel (with offset for midpoint)
    // -2 -1  0
    // -1  1  1
    //  0  1  2
    QVector<QVector<int>> kernel = {
        { -2, -1,  0 },
        { -1,  1,  1 },
        {  0,  1,  2 }
    };
    // Add offset=128 to shift mid-values into visible range
    return applyConvolution(image, kernel, /*divisor*/ 1, /*offset*/ 128, 1, 1);
}

} // namespace Filters
