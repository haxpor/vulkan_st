#include "VkBase.h"

const int WIDTH = 800;
const int HEIGHT = 600;

class TriangleApp : public VkBase {
    
};

int main() {
    TriangleApp app;
    app.init(WIDTH, HEIGHT, "Vulkan - Triangle");
    app.run();

    return 0;
}
