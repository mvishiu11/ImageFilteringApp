#ifndef FILTERS_H
#define FILTERS_H

#include <QImage>

// A namespace to store all filter-related functions
namespace Filters {

// Functional filters
QImage invert(const QImage &image);
QImage adjustBrightness(const QImage &image, int delta);
QImage adjustContrast(const QImage &image, double factor);
QImage adjustGamma(const QImage &image, double gammaValue);

// Convolution filters
QImage blur3x3(const QImage &image);
QImage gaussianBlur3x3(const QImage &image);
QImage sharpen3x3(const QImage &image);
QImage edgeDetect3x3(const QImage &image);
QImage emboss3x3(const QImage &image);

} // namespace Filters

#endif // FILTERS_H
