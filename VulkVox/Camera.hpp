#ifndef VV_CAMERA
#define VV_CAMERA

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>

struct Camera {

    glm::vec3 eye;
    glm::vec3 up;
    glm::mat4 model;
    glm::vec3 center;
    float fovy;
    float zNear;
    float zFar;

    Camera() : 
        center(glm::vec3(0.0f, 0.0f, 0.0f)),
        eye(glm::vec3(10.0f, 10.0f, 15.0f)),
        model(glm::mat4(1.0f)),
        up(glm::vec3(0.0f, 0.0f, 1.0f)),
        fovy(45.0f),
        zNear(0.01f),
        zFar(1000.0f)
    {
    }

    void pan(float x, float y) {
        eye[0] += x;
        eye[1] += y;
    }

    void zoom(float amount) {
        eye[0] += amount;
        eye[1] += amount;
        eye[2] += amount;
    }

};

#endif
