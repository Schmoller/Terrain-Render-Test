#include "theme.hpp"

namespace Theme {

void normal(Vector::Object &object) {
    object.setFill({ 0.23, 0.53, 0.89, 0.25 });
    object.setStroke({ 0.23, 0.53, 0.89, 1 });
    object.setStrokePosition(Vector::StrokePosition::Center);
    object.setStrokeWidth(1);
}

void error(Vector::Object &object) {
    object.setFill({ 0.86, 0.04, 0.13, 0.25 });
    object.setStroke({ 0.86, 0.04, 0.13, 1 });
    object.setStrokePosition(Vector::StrokePosition::Center);
    object.setStrokeWidth(1);
}

void good(Vector::Object &object) {
    object.setFill({ 0.15, 0.89, 0.22, 0.25 });
    object.setStroke({ 0.15, 0.89, 0.22, 1 });
    object.setStrokePosition(Vector::StrokePosition::Center);
    object.setStrokeWidth(1);
}

void informational(Vector::Object &object) {
    object.setFill({ 0, 0, 0, 0 });
    object.setStroke({ 0.04, 0.73, 0.86, 1 });
    object.setStrokePosition(Vector::StrokePosition::Center);
    object.setStrokeWidth(0.4);
}

}