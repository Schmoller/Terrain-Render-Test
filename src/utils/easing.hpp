#pragma once

namespace Easing {
template<typename T>
// Quadratic ease-out http://gizma.com/easing/#quad2
T quadraticOut(const T &start, const T &end, float progress) {
    auto change = end - start;
    return -change * progress * (progress - 2) + start;
}

}