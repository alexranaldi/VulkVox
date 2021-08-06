#ifndef VV_VERTEX
#define VV_VERTEX

#include <array>
#include <vector>
#include <cstdint>
#include <random>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>

#include "Colors.hpp"
#include "Constants.hpp"

//#include "VulkanMemoryHelpers.hpp"

#include "VulkanPrimary.hpp"

constexpr int FLOAT_MIN = 0;
constexpr int FLOAT_MAX = 1;

using VertexIndex = uint32_t;

static glm::vec3 getRandomColor() {
    float a = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
    float b = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
    float c = FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
    glm::vec3 ret = {a,b,c};
    return ret;
}

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;

    Vertex() {
    }

    Vertex(const glm::vec3 p, const glm::vec3 c) : pos(p), color(c) {
    }

    void translate(const glm::vec3 o) {
        pos[0] += o[0];
        pos[1] += o[1];
        pos[2] += o[2];
    }

    void scale(const float f) {
        pos[0] *= f;
        pos[1] *= f;
        pos[2] *= f;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

constexpr std::size_t VERTEX_SIZE = sizeof(Vertex);

struct Quad {
public:
    Quad(const Vertex& v1, const Vertex& v2, const Vertex& v3, const Vertex& v4){
        vertices = {
            v1, v2, v3, v4
        };
    }
    // copy constructor
    Quad(const Quad& q) :
        vertices(q.getVertices()) {
    }
    void translate(glm::vec3 o) {
        for (auto& v : vertices) {
            v.translate(o);
        }
    }
    void scale(const float f) {
        for (auto& v : vertices) {
            v.scale(f);
        }
    }
    std::vector<Vertex> getVertices() const {
        return vertices;
    }
    std::vector<VertexIndex> getVertexIndices() const {
        const std::vector<VertexIndex> vertexIndices = {
            0, 1, 3, 2, 3, 1,
        };
        return vertexIndices;
    }
    void setColor(glm::vec3 color1) {
        vertices[0].color = color1;
        vertices[1].color = color1;
        vertices[2].color = color1;
        vertices[3].color = color1;
    }

private:
    std::vector<Vertex> vertices;

};

struct Cube {
    std::vector<Vertex> vertices;

    std::array<Vertex,24> verticesArray;

    std::vector<VertexIndex> vertexIndices;

    const std::size_t CUBE_VERTEX_BUFFER_SIZE = 24 * sizeof(Vertex);
    const std::size_t CUBE_INDEX_BUFFER_SIZE = 6 * 6 * sizeof(VertexIndex);

    bool isDirty = true;

    Cube(const Quad& s1, const Quad& s2, const Quad& s3, const Quad& s4, const Quad& s5, const Quad& s6) {
        std::vector<Vertex> v1 = s1.getVertices();
        std::vector<Vertex> v2 = s2.getVertices();
        std::vector<Vertex> v3 = s3.getVertices();
        std::vector<Vertex> v4 = s4.getVertices();
        std::vector<Vertex> v5 = s5.getVertices();
        std::vector<Vertex> v6 = s6.getVertices();

        size_t destIdx = 0;
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v1[k];
        }
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v2[k];
        }
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v3[k];
        }
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v4[k];
        }
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v5[k];
        }
        for (size_t k = 0; k < 4; ++k) {
            verticesArray[destIdx++] = v6[k];
        }
    }

    Cube(const Cube& c) {
        for (size_t k = 0; k < VERTICES_PER_CUBE; ++k) {
            verticesArray[k] = c.verticesArray[k];
        }
        vertexIndices.reserve(VERTEX_INDICES_PER_CUBE);
    }

    void translate(const glm::vec3 o) {
        for (auto& v : verticesArray) {
            v.translate(o);
        }
    }

    void scale(const float f) {
        for (auto& v : verticesArray) {
            v.scale(f);
        }
    }

    void colorRandomly() {
        
        for (size_t index = 0; index < VERTICES_PER_CUBE; index+=4) {
            auto c = getRandomColor();
            verticesArray[index+0].color = c;
            verticesArray[index+1].color = c;
            verticesArray[index+2].color = c;
            verticesArray[index+3].color = c;
        }
        isDirty = true;
    }

    void copyVertices(std::vector<Vertex>& dstVertices, std::vector<VertexIndex>& dstCubeVertexIndices, size_t cubeIndex) {

        if (!isDirty) { return; }

        // copy vertices
        size_t offset = cubeIndex * VERTICES_PER_CUBE;
        for (size_t k = 0; k < VERTICES_PER_CUBE; ++k) {
            dstVertices[offset + k] = verticesArray[k];
        }

        // copy vertex indices
        auto srcVertexIndices = getVertexIndices(cubeIndex * 24);
        offset = cubeIndex * VERTEX_INDICES_PER_CUBE;
        for (size_t k = 0; k < VERTEX_INDICES_PER_CUBE; ++k) {
            dstCubeVertexIndices[offset + k] = srcVertexIndices[k];
        }

        isDirty = false;

    }

    std::vector<VertexIndex> gvi() const {
        return {
            0, 1, 3, 2, 3, 1,
        };
    }

    std::vector<VertexIndex> getVertexIndices(VertexIndex offset) {
        if (VERTEX_INDICES_PER_CUBE == vertexIndices.size()) {
            return vertexIndices;
        }

        std::vector<VertexIndex> v1 = gvi();
        std::vector<VertexIndex> v2 = gvi();
        std::vector<VertexIndex> v3 = gvi();
        std::vector<VertexIndex> v4 = gvi();
        std::vector<VertexIndex> v5 = gvi();
        std::vector<VertexIndex> v6 = gvi();

        for (std::size_t k = 0; k < v1.size(); ++k) {
            vertexIndices.push_back(v1[k] + offset);
        }
        
        offset += 4;
        for (std::size_t k = 0; k < v2.size(); ++k) {
            vertexIndices.push_back(v2[k] + offset);
        }

        offset += 4;
        for (std::size_t k = 0; k < v3.size(); ++k) {
            vertexIndices.push_back(v3[k] + offset);
        }

        offset += 4;
        for (std::size_t k = 0; k < v4.size(); ++k) {
            vertexIndices.push_back(v4[k] + offset);
        }

        offset += 4;
        for (std::size_t k = 0; k < v5.size(); ++k) {
            vertexIndices.push_back(v5[k] + offset);
        }

        offset += 4;
        for (std::size_t k = 0; k < v6.size(); ++k) {
            vertexIndices.push_back(v6[k] + offset);
        }
        return vertexIndices;
        
    }

};

struct WorldObjects
{
public:

    std::vector<VertexIndex> getVertexIndices() {
        std::vector<VertexIndex> indices;
        indices.reserve(36 * size());
        for (auto& c : cubes) {
            auto v = c.getVertexIndices(indices.size() / 6 * 4);
            std::copy(v.begin(), v.end(), std::back_inserter(indices));
        }
        return indices;
    }

    std::size_t size() const {
        return cubes.size();
    }

    void clear() {
        cubes.clear();
    }

    void append(const Cube& cube) {
        cubes.push_back(cube);
    }

    std::vector<Cube> cubes;

};

#endif
