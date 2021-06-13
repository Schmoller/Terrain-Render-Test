#pragma once

#include "event.hpp"
#include "../vector/common.hpp"

#include <string>
#include <functional>
#include <glm/glm.hpp>

class ToolBase {
    friend class Scene;
    typedef std::function<void(ToolBase *, bool)> ActivationCallback;
public:
    virtual ~ToolBase() = default;

    virtual const char *getName() = 0;

    virtual void onDeactivate() {}

    virtual void onActivate() {};

    virtual void onMouseDown(const ToolMouseEvent &event) {};

    virtual void onMouseMove(const ToolMouseEvent &event, double delta) {};

    virtual void onMouseUp(const ToolMouseEvent &event) {};

    virtual std::shared_ptr<Vector::Object> createHighlight() { return {}; };

    virtual void drawToolbarTab() = 0;
protected:
    /**
     * Sets the tool as active
     */
    void activate();

    /**
     * Sets the tool as inactive
     */
    void deactivate();
private:
    void setStateCallback(const ActivationCallback &);

    ActivationCallback activationCallback;
};



