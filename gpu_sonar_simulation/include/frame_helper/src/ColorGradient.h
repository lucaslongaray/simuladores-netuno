#ifndef COLORGRADIENT_H
#define COLORGRADIENT_H

#include <list>
#include <stdexcept>
#include "ColorGradientTypes.h"

namespace frame_helper {

class ColorGradient {

    struct ColorPoint {     // Internal class used to store colors at different points in the gradient.
        float r, g, b;      // Red, green and blue values of our color.
        float val;          // Position of our color along the gradient (between 0 and 1).
        ColorPoint(float red, float green, float blue, float value) :
                r(red), g(green), b(blue), val(value) {
        }
    };

private:
    std::list<ColorPoint> color;      // An array of color points in ascending value.

public:
    ColorGradient() { }
    void addColorPoint(const float red, const float green, const float blue, const float value);
    void clearGradient();
    void getColorAtValue(const float value, float &red, float &green, float &blue) const;
    void colormapSelector(const ColorGradientType type);
};

}

#endif
