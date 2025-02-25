#include "filters.h"
#include <QtMath>
#include <algorithm>

namespace { // An anonymous namespace to group the convolution helpers

/**
 * @brief applyConvolution3x3
 * Applies a 3×3 convolution on the input image using the provided kernel, divisor, and offset.
 *
 * @param image     The input image (converted to RGB32 internally).
 * @param kernel    A 3×3 integer kernel.
 * @param divisor   The value used to divide the summed pixel contributions.
 * @param offset    A bias added after division (useful for emboss or custom shifts).
 * @return          A new QImage with the convolution applied.
 */
QImage applyConvolution3x3(const QImage &image, const int kernel[3][3],
                           int divisor, int offset = 0)
{
    QImage src = image.convertToFormat(QImage::Format_RGB32);
    QImage dst(src.width(), src.height(), QImage::Format_RGB32);

    // For safety, avoid dividing by 0
    if (divisor == 0) divisor = 1;

    // Loop over each pixel, skipping the borders to avoid going out of range
    for (int y = 0; y < src.height(); ++y) {
        for (int x = 0; x < src.width(); ++x) {
            // Edges: replicate nearest pixel or skip?
            // For simplicity, if we are at border, use original pixel
            if (x == 0 || x == src.width() - 1 || y == 0 || y == src.height() - 1) {
                dst.setPixel(x, y, src.pixel(x, y));
                continue;
            }

            // Accumulators
            int sumR = 0, sumG = 0, sumB = 0;

            // Apply the kernel
            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    int pixelX = x + kx;
                    int pixelY = y + ky;
                    QRgb pixel = src.pixel(pixelX, pixelY);

                    int red   = qRed(pixel);
                    int green = qGreen(pixel);
                    int blue  = qBlue(pixel);

                    int factor = kernel[ky + 1][kx + 1];
                    sumR += red   * factor;
                    sumG += green * factor;
                    sumB += blue  * factor;
                }
            }

            // Divide, clip
            int outR = std::clamp((sumR / divisor) + offset, 0, 255);
            int outG = std::clamp((sumG / divisor) + offset, 0, 255);
            int outB = std::clamp((sumB / divisor) + offset, 0, 255);

            // Set the new pixel
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
    int kernel[3][3] = {
        { 1, 1, 1 },
        { 1, 1, 1 },
        { 1, 1, 1 }
    };
    return applyConvolution3x3(image, kernel, /*divisor*/ 9, /*offset*/ 0);
}

QImage gaussianBlur3x3(const QImage &image) {
    // Basic 3x3 Gaussian kernel
    // 1 2 1
    // 2 4 2
    // 1 2 1
    int kernel[3][3] = {
        { 1, 2, 1 },
        { 2, 4, 2 },
        { 1, 2, 1 }
    };
    return applyConvolution3x3(image, kernel, /*divisor*/ 16, /*offset*/ 0);
}

QImage sharpen3x3(const QImage &image) {
    // A common sharpen kernel
    //  0 -1  0
    // -1  5 -1
    //  0 -1  0
    int kernel[3][3] = {
        {  0, -1,  0 },
        { -1,  5, -1 },
        {  0, -1,  0 }
    };
    return applyConvolution3x3(image, kernel, /*divisor*/ 1, /*offset*/ 0);
}

QImage edgeDetect3x3(const QImage &image) {
    // A simple edge detection kernel
    //  0  1  0
    //  1 -4  1
    //  0  1  0
    int kernel[3][3] = {
        {  0,  1,  0 },
        {  1, -4,  1 },
        {  0,  1,  0 }
    };
    return applyConvolution3x3(image, kernel, /*divisor*/ 1, /*offset*/ 0);
}

QImage emboss3x3(const QImage &image) {
    // A basic emboss kernel (with offset for midpoint)
    // -2 -1  0
    // -1  1  1
    //  0  1  2
    int kernel[3][3] = {
        { -2, -1,  0 },
        { -1,  1,  1 },
        {  0,  1,  2 }
    };
    // Add offset=128 to shift mid-values into visible range
    return applyConvolution3x3(image, kernel, /*divisor*/ 1, /*offset*/ 128);
}

} // namespace Filters
