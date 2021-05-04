#include <iostream>
#include "scene.hpp"

int main() {
    Scene scene;

    try {
        scene.initialize();
    } catch (std::exception &ex) {
        std::cerr << "Failed to load engine: " << ex.what() << std::endl;
        return 1;
    }

    scene.run();

    return 0;
}
