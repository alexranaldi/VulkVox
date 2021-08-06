#include <chrono>
#include <random>
#include <ctime>
#include <thread>

#include "VulkanBootstrapper.hpp"
#include "VulkanPrimary.hpp"

#include "Vertex.hpp"
#include "CommandState.hpp"
#include "Camera.hpp"

uint64_t framesDrawnCount;

uint64_t lastFramesDrawnCount;

VulkanPrimary vkprim;

WorldObjects wo;

CommandState cs(vkprim);

void logicThreadFcn() {
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        for (auto& c : wo.cubes) {
            c.colorRandomly();
        }
    }
}

void fpsThreadFcn() {
    while(1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ONE_SECOND_IN_MS));
        uint64_t fps = framesDrawnCount - lastFramesDrawnCount;
        lastFramesDrawnCount = framesDrawnCount;
        std::cout << "FPS Last Second: " << fps << std::endl;
        std::cout << "Total Cubes: " << wo.size() << std::endl;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    cs.onMouseScroll(xoffset, yoffset);
    std::cout << "scroll " << std::to_string(yoffset) << std::endl;
    std::cout << "eye[2] " << std::to_string(vkprim.camera.eye[2]) << std::endl;
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    std::cout << "Cursor X: " << xpos << std::endl;
    std::cout << "Cursor Y: " << ypos << std::endl;
    cs.onMouseMove(xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        std::cout << "LEFT MOUSE PRESS" << std::endl;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        cs.onMouseRightClick();
        std::cout << "RIGHT MOUSE PRESS" << std::endl;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        cs.onMouseRightRelease();
        std::cout << "RIGHT MOUSE RELEASE" << std::endl;
    }
}

int main() {

    std::srand(std::time(nullptr));

    framesDrawnCount = 0;
    lastFramesDrawnCount = 0;

    //const std::vector<Vertex> vertices = makeQuad({ 0.0f,   1.0f,  0.0f }, { 0.0f,   0.0f,  0.0f}, { 1.0f,   1.0f,  0.0f }, { 1.0f,   0.0f,  0.0f });

    const Quad q1 = Quad(Vertex({ 0.0f,   1.0f,  0.0f }, RED), Vertex({ 0.0f,   0.0f,  0.0f }, RED), Vertex({ 1.0f,   0.0f,  0.0f }, RED), Vertex({ 1.0f,   1.0f,  0.0f }, RED));
    const Quad q2 = Quad(Vertex({ 0.0f,   1.0f,  1.0f }, BLUE), Vertex({ 0.0f,   0.0f,  1.0f }, BLUE), Vertex({ 1.0f,   0.0f,  1.0f }, BLUE), Vertex({ 1.0f,   1.0f,  1.0f }, BLUE));
    const Quad q3 = Quad(Vertex({ 0.0f,   0.0f,  0.0f }, GREEN), Vertex({ 0.0f,   1.0f,  0.0f }, GREEN), Vertex({ 0.0f,   1.0f,  1.0f }, GREEN), Vertex({ 0.0f,   0.0f,  1.0f }, GREEN));
    const Quad q4 = Quad(Vertex({ 1.0f,   0.0f,  0.0f }, WHITE), Vertex({ 1.0f,   1.0f,  0.0f }, WHITE), Vertex({ 1.0f,   1.0f,  1.0f }, WHITE), Vertex({ 1.0f,   0.0f,  1.0f }, WHITE));
    const Quad q5 = Quad(Vertex({ 1.0f,   1.0f,  0.0f }, MAGENTA), Vertex({ 0.0f,   1.0f,  0.0f }, MAGENTA), Vertex({ 0.0f,   1.0f,  1.0f }, MAGENTA), Vertex({ 1.0f,   1.0f,  1.0f }, MAGENTA));
    const Quad q6 = Quad(Vertex({ 1.0f,   0.0f,  0.0f }, ORANGE), Vertex({ 0.0f,   0.0f,  0.0f }, ORANGE), Vertex({ 0.0f,   0.0f,  1.0f }, ORANGE), Vertex({ 1.0f,   0.0f,  1.0f }, ORANGE));

    Cube c(q1, q2, q3, q4, q5, q6);
   // c.scale(0.1);
    wo.append(c);

    for (int z = 0; z < 6; ++z)
    {
        for (int x = 1; x < 20; ++x)
        {
            for (int y = 1; y < 20; ++y)
            {
                const float k = (float)(x) ;//* 0.1;
                const float a = (float)(y) ; // * 0.1;
                const float b = (float)(z); // * 0.1;

                Cube c2(c);
                c2.translate({ a, k, b });
                c2.colorRandomly();
                wo.append(c2);

                Cube cx(c);
                cx.translate({ a, -k, b });
                cx.colorRandomly();
                wo.append(cx);
            
                Cube c4(c);
                c4.translate({ -a, -k, b });
                c4.colorRandomly();
                wo.append(c4);
            
                Cube c3(c);
                c3.translate({ -a, k, b });
                c3.colorRandomly();
                wo.append(c3);
            }
        }
    }

    std::thread fpsThread(fpsThreadFcn);
    std::thread logicThread(logicThreadFcn);

    try {
        vkprim.bootstrap();
    }
    catch (const std::exception& e) {
        std::cerr << "Failuring bootstrapping Vulkan!" << std::endl;
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    vkprim.cubeVertices.reserve(MAX_CUBES * VERTICES_PER_CUBE);
    vkprim.cubeVertexIndices.reserve(MAX_CUBES * VERTEX_INDICES_PER_CUBE);

    vkprim.setWorldObjects(&wo);

    glfwSetScrollCallback(vkprim.window, scroll_callback);
    glfwSetMouseButtonCallback(vkprim.window, mouse_button_callback);
    glfwSetInputMode(vkprim.window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetCursorPosCallback(vkprim.window, cursor_position_callback);

    try {
        while (!glfwWindowShouldClose(vkprim.window)) {
            glfwPollEvents();
            vkprim.drawFrame();
            framesDrawnCount++;
        }
        vkDeviceWaitIdle(vkprim.getDevice());
        vkprim.cleanup();
        fpsThread.join();
        logicThread.join();

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    wo.clear();

    return EXIT_SUCCESS;
}
