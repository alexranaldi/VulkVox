#ifndef VV_COMMANDSTATE
#define VV_COMMANDSTATE

enum class MouseState {
    None = 0,
    RightPress = 1,
};

struct CommandState {

    MouseState mouseState;
    int commandState;
    VulkanPrimary& vkprim;
    double lastMouseX;
    double lastMouseY;

    CommandState(VulkanPrimary& vkprimIn) :
        vkprim(vkprimIn),
        mouseState(MouseState::None),
        commandState(0) {
    }

    void onMouseRightClick() {
        glfwGetCursorPos(vkprim.window, &lastMouseX, &lastMouseY);
        mouseState = MouseState::RightPress;
    }

    void onMouseRightRelease() {
        mouseState = MouseState::None;
    }

    void onMouseMove(double x, double y) {
        if (MouseState::RightPress == mouseState) {
            double dx = x - lastMouseX;
            double dy = y - lastMouseY;
            double ax = dx / 6;
            double ay = dy / 6;
            vkprim.camera.pan(ax, ay);
            lastMouseX = x;
            lastMouseY = y;
        }
    }

    void onMouseScroll(double x, double y) {
        vkprim.camera.zoom(y);
    }

};

#endif
