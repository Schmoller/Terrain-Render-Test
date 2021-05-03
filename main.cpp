#include <iostream>
#include <tech-core/engine.hpp>

int main() {
    Engine::RenderEngine engine;

    try {
        engine.initialize("Terrain test");
    } catch (std::exception &ex) {
        std::cerr << "Failed to load engine: " << ex.what() << std::endl;
        return 1;
    }

    auto &input = engine.getInputManager();

    while (engine.beginFrame()) {
        if (input.isPressed(Engine::Key::eEscape)) {
            break;
        }
        engine.render();

    }


    return 0;
}
