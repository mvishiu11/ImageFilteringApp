#include "ditheringandquantization.h"
#include <QColor>
#include <QMap>
#include <QPair>
#include <QtMath>
#include <algorithm>

namespace {
/* --- Helper: Predefined Threshold Matrices --- */
static QVector<QVector<int>> getThresholdMatrix(int size) {
  if (size == 2) {
    return {{0, 2}, {3, 1}};
  }
  if (size == 3) {
    return {{6, 8, 4}, {1, 0, 3}, {5, 2, 7}};
  }
  if (size == 4) {
    return {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};
  }
  if (size == 6) {
    return {{0, 32, 8, 40, 2, 34},   {48, 16, 56, 24, 50, 18},
            {12, 44, 4, 36, 14, 46}, {60, 28, 52, 20, 62, 30},
            {3, 35, 11, 43, 1, 33},  {51, 19, 59, 27, 49, 17}};
  }
  return getThresholdMatrix(2);
}
} // namespace

namespace DitheringAndQuantization {

/* --- Ordered Dithering --- */
QImage applyOrderedDithering(const QImage &image, int thresholdMapSize,
                             int levelsPerChannel) {
  if (levelsPerChannel < 2)
    levelsPerChannel = 2; // At least 2 levels.

  QVector<QVector<int>> thresholdMatrix = getThresholdMatrix(thresholdMapSize);
  int matrixMax = thresholdMapSize * thresholdMapSize;
  // Formula:
  // v_norm = v / 255 * levelsPerChannel.
  // q = floor(v_norm)
  // frac = v_norm - q.
  // T = (thresholdMatrix[x mod N][y mod N] + 0.5) / (matrixMax)
  // If (frac > T) then q++.
  // Output = clamp(q, 0, levelsPerChannel - 1) scaled back to 0-255.

  QImage src = image.convertToFormat(QImage::Format_RGB32);
  QImage dst(src.size(), QImage::Format_RGB32);
  int width = src.width();
  int height = src.height();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      QColor origColor(src.pixel(x, y));
      int newR = 0, newG = 0, newB = 0;
      // Process each channel:
      for (int channel = 0; channel < 3; ++channel) {
        int v = 0;
        if (channel == 0)
          v = origColor.red();
        else if (channel == 1)
          v = origColor.green();
        else if (channel == 2)
          v = origColor.blue();

        double v_norm = (v / 255.0) * levelsPerChannel; // value in [0, levels]
        int q = int(floor(v_norm));
        double frac = v_norm - q;
        // Get threshold from matrix:
        int i = x % thresholdMapSize;
        int j = y % thresholdMapSize;
        double T = (thresholdMatrix[j][i] + 0.5) / double(matrixMax);
        if (frac > T) {
          q++;
        }
        if (q < 0)
          q = 0;
        if (q >= levelsPerChannel)
          q = levelsPerChannel - 1;
        // Map q to 0..255:
        int newVal = (levelsPerChannel > 1)
                         ? int(q * 255.0 / (levelsPerChannel - 1))
                         : 0;
        if (channel == 0)
          newR = newVal;
        else if (channel == 1)
          newG = newVal;
        else if (channel == 2)
          newB = newVal;
      }
      dst.setPixel(x, y, qRgb(newR, newG, newB));
    }
  }
  return dst;
}

/* --- Ordered Dithering in YCbCr --- */
QImage applyOrderedDitheringInYCbCr(const QImage &image, int thresholdMapSize,
                                    int levelsY) {
  if (levelsY <= 2)
    levelsY = 3;
  else if (levelsY % 2 == 0)
    levelsY = levelsY + 1;

  QVector<QVector<int>> thresholdMatrix = getThresholdMatrix(thresholdMapSize);
  int matrixSize = thresholdMapSize;
  int matrixMax = matrixSize * matrixSize;

  QImage src = image.convertToFormat(QImage::Format_RGB32);
  QImage dst(src.size(), QImage::Format_RGB32);
  int width = src.width();
  int height = src.height();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      QColor origColor(src.pixel(x, y));
      // Convert RGB to YCbCr
      double R = origColor.red();
      double G = origColor.green();
      double B = origColor.blue();
      double Y_val = 0.299 * R + 0.587 * G + 0.114 * B;
      double Cb = 128 - 0.168736 * R - 0.331264 * G + 0.5 * B;
      double Cr = 128 + 0.5 * R - 0.418688 * G - 0.081312 * B;

      // --- Apply Ordered Dithering on the Y Channel ---
      double y_norm = (Y_val / 255.0) * levelsY;
      int q = int(floor(y_norm));
      double frac = y_norm - q;
      int i = x % matrixSize;
      int j = y % matrixSize;
      double T = (thresholdMatrix[j][i] + 0.5) / double(matrixMax);
      if (frac > T)
        q++;
      q = std::clamp(q, 0, levelsY - 1);
      double newY = (levelsY > 1) ? (q * 255.0 / (levelsY - 1)) : 0;

      // --- Convert YCbCr back to RGB ---
      int newR = int(newY + 1.402 * (Cr - 128));
      int newG = int(newY - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128));
      int newB = int(newY + 1.772 * (Cb - 128));
      newR = std::clamp(newR, 0, 255);
      newG = std::clamp(newG, 0, 255);
      newB = std::clamp(newB, 0, 255);

      dst.setPixel(x, y, qRgb(newR, newG, newB));
    }
  }
  return dst;
}

/* --- Popularity Quantization --- */
QImage applyPopularityQuantization(const QImage &image, int numColors) {
  QImage src = image.convertToFormat(QImage::Format_RGB32);
  int width = src.width();
  int height = src.height();

  // Step 1: Count frequencies of colors.
  QMap<QRgb, int> colorFrequency;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      QRgb pixel = src.pixel(x, y);
      colorFrequency[pixel] += 1;
    }
  }

  // Step 2: Sort colors by frequency.
  QVector<QPair<QRgb, int>> freqList;
  for (auto it = colorFrequency.begin(); it != colorFrequency.end(); ++it) {
    freqList.append(qMakePair(it.key(), it.value()));
  }
  std::sort(freqList.begin(), freqList.end(),
            [](const QPair<QRgb, int> &a, const QPair<QRgb, int> &b) {
              return a.second > b.second;
            });

  // Step 3: Select the top numColors.
  QVector<QRgb> palette;
  int count = qMin(numColors, freqList.size());
  for (int i = 0; i < count; ++i) {
    palette.append(freqList[i].first);
  }
  if (palette.isEmpty()) {
    return src;
  }

  // Step 4: For each pixel, find the nearest color in the palette.
  QImage dst(src.size(), QImage::Format_RGB32);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      QColor origColor(src.pixel(x, y));
      int bestDist = 1e9;
      QRgb bestColor = palette.first();
      for (QRgb palColor : palette) {
        QColor c(palColor);
        int dr = origColor.red() - c.red();
        int dg = origColor.green() - c.green();
        int db = origColor.blue() - c.blue();
        int dist = dr * dr + dg * dg + db * db;
        if (dist < bestDist) {
          bestDist = dist;
          bestColor = palColor;
        }
      }
      dst.setPixel(x, y, bestColor);
    }
  }
  return dst;
}

} // namespace DitheringAndQuantization
