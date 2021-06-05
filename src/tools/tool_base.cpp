#include "tool_base.hpp"

void ToolBase::setStateCallback(const ActivationCallback &callback) {
    activationCallback = callback;
}

void ToolBase::activate() {
    if (activationCallback) {
        activationCallback(this, true);
    }
}

void ToolBase::deactivate() {
    if (activationCallback) {
        activationCallback(this, false);
    }
}