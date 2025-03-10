# Image Filtering App

## Overview

The Image Filtering App is a cross-platform desktop application built using C++ and the Qt framework. It allows users to load, view, and process images using a variety of filters. The application emphasizes manual image filtering operations with pixel-level control and offers a professional, user-friendly interface.

## Features

- **Image Loading & Display**
  - Load color images in various formats (e.g., PNG, JPEG, BMP).
  - Display the original image alongside filtered results.
  
- **Functional Filters**
  - **Inversion:** Invert the colors of an image.
  - **Brightness Correction:** Adjust brightness using a custom offset.
  - **Contrast Enhancement:** Adjust contrast with a configurable factor.
  - **Gamma Correction:** Apply gamma correction.
  - **Functional Editor:** A dedicated 256×256 canvas where users can create and edit piecewise-linear lookup tables (LUTs) for custom functional filtering. The editor supports preset curves for brightness, contrast, and inversion, and allows manual modification of the function.

- **Convolution Filters**
  - **Blur, Gaussian Blur, Sharpen, Edge Detection, Emboss:** Apply common convolution filters with preset kernels.
  - **Convolution Editor:** An interactive dockable widget that lets users select kernel size, edit coefficients via a table, set divisor and offset values (with an option for automatic divisor calculation), and choose the anchor point. Preset buttons provide quick access to standard filters.

- **Morphological Filters**
  - **Erosion and Dilation:** Apply erosion and dilation filters that process each color channel separately.

- **Advanced Features**
  - Combine multiple filters sequentially.
  - Revert filtered images back to their original state.
  - Save the processed image to a file.
  - Custom styling via Qt Style Sheets for a modern, polished look.

## Requirements

- **Development Environment:**
  - C++ compiler supporting C++11 (or later).
  - Qt 6 framework.
- **Platform:**
  - Ubuntu, Windows, macOS (cross-platform)

## Build & Installation

1. **Clone the Repository:**

   ```bash
   git clone https://github.com/yourusername/ImageFilteringApp.git
   cd ImageFilteringApp
   ```

2. **Configure the Project:**
   - If using CMake:
     ```bash
     mkdir build && cd build
     cmake ..
     ```
   - Or, if using Qt Creator, open the project file (.pro or CMakeLists.txt) in Qt Creator.

3. **Build the Application:**
   - Run the build command (e.g., `make` or use the Qt Creator build tools).

4. **Run the Application:**
   - Execute the generated binary (e.g., `./ImageFilteringApp`).

## Usage

- **Load an Image:**  
  Use the File menu or the provided "Load" button to select an image file.

- **Apply Filters:**  
  Select one or more filters from the available options.  
  - Use the functional editor to create custom transformations by editing the LUT curve.  
  - Use the convolution editor to apply convolution-based effects, with options to choose preset filters or manually adjust parameters.
  - Use the morphological tools (erosion/dilation) for additional image processing effects.

- **Combine & Save:**  
  Filters can be applied in succession.  
  Use the "Reset" option to revert changes and "Save" to write the final output to a file.

## Customization

- **Styling:**  
  The application uses Qt Style Sheets (QSS) to define its visual appearance. You can modify `style.qss` to change colors, fonts, and other UI elements.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with [Qt](https://www.qt.io/)
- Inspired by typical computer graphics and image processing courses.